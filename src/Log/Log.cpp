/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/09 14:28:39 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"
#include "Display.hpp"
#include "Settings.hpp"
#include "Thread.hpp"

#pragma region Variables

	pthread_mutex_t				Log::mutex;																	//	Mutex for synchronizing access to shared resources

	pthread_t					Log::_thread;																//	Thread used for processing log entries
	bool						Log::_terminate = false;													//	Flag the thread to finish
	std::queue <Log::LogInfo>	Log::_logs;																	//	Queue container with logs that need to be processed

	const size_t				Log::MEM_MAXSIZE = 200;														//	Maximum number of logs for each memory log
	long						Log::LOCAL_MAXSIZE = 1 * 1024 * 1024 * 1024;								//	Maximum size of the log in disk (default to 10 MB | 0 MB = dont truncate | Max 10 MB)

	#pragma region LogInfo

		#pragma region Constructors

			Log::LogInfo::LogInfo(std::string & _msg, int _type, VServer * _VServ, std::string _path) : msg(_msg), type(_type), VServ(_VServ), path(_path) {}

		#pragma endregion

		#pragma region Overloads

			Log::LogInfo &	Log::LogInfo::operator=(const LogInfo & rhs) {
				if (this != &rhs) { msg = rhs.msg; type = rhs.type; VServ = rhs.VServ; path = rhs.path; }
				return (*this);
			}

			bool			Log::LogInfo::operator==(const LogInfo & rhs) {
				return (msg == rhs.msg && type == rhs.type && VServ == rhs.VServ && path == rhs.path);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Log

	#pragma region Constructors

		Log::Log() {}
		Log::Log(const Log & src) { *this = src; }
		Log::~Log() { clear(); }

	#pragma endregion

	#pragma region Overloads

		Log &	Log::operator=(const Log & rhs) { if (this != &rhs) { access = rhs.access; error = rhs.error; both = rhs.both; } return (*this); }
		bool	Log::operator==(const Log & rhs) const { return (access == rhs.access && error == rhs.error && both == rhs.both); }

	#pragma endregion

	#pragma region Memory Logs

		#pragma region Add

			void Log::access_add(const std::string & msg) {
				if (access.size() == MEM_MAXSIZE) access.pop_front();
					access.push_back(msg);
			}

			void Log::error_add(const std::string & msg) {
				if (error.size() == MEM_MAXSIZE) error.pop_front();
					error.push_back(msg);
			}

			void Log::both_add(const std::string & msg) {
				if (both.size() == MEM_MAXSIZE) both.pop_front();
					both.push_back(msg);
			}

		#pragma endregion

		#pragma region Clear

			void Log::clear() {
				access.clear(); error.clear(); both.clear();
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Static
	
	#pragma region Logs

		#pragma region Log to Memory

			void Log::log_to_memory(std::string msg, int type, VServer * VServ) {
				if (msg.empty()) return ;
				if (Settings::check_only || !Display::isRawMode() || Display::ForceRawModeDisabled) {
					if (Settings::check_only && Settings::loaded == false) {
						if (msg.size() > 24) msg = msg.substr(24); else msg = "";
					}
					if (!Display::background && !msg.empty()) std::cout << " " << msg << NC << std::endl;
				}

				Thread::mutex_set(mutex, Thread::MTX_LOCK);
				if (type == MEM_ACCESS || type == BOTH_ACCESS) {
					Settings::global.log.both_add(msg); Settings::global.log.access_add(msg);
					if (VServ) { VServ->log.both_add(msg); VServ->log.access_add(msg); }
				}
				if (type == MEM_ERROR || type == BOTH_ERROR) {
					Settings::global.log.both_add(msg); Settings::global.log.error_add(msg);
					if (VServ) { VServ->log.both_add(msg); VServ->log.error_add(msg); }
				}
				Thread::mutex_set(mutex, Thread::MTX_UNLOCK);
			}

		#pragma endregion

		#pragma region Log to File

			void Log::log_to_file(const std::string & msg, std::string path) {
				if (msg.empty() || path.empty()) return;

				std::ofstream outfile; outfile.open(path.c_str(), std::ios_base::app);
				if (outfile.is_open()) {
					outfile << msg; outfile.flush();
					outfile.close();
				}
			}

		#pragma endregion

		#pragma region Process Logs

			void Log::process_logs() {
				std::queue <Log::LogInfo> logs;

				Thread::mutex_set(mutex, Thread::MTX_LOCK);
				if (!_logs.empty()) std::swap(logs, _logs);
				Thread::mutex_set(mutex, Thread::MTX_UNLOCK);

				if (logs.empty()) return;

				std::map<std::string, std::string> logMap;

				while (!logs.empty()) {
					Log::LogInfo log = logs.front(); logs.pop();
					
					Utils::trim(log.msg); if (log.msg.empty()) continue;
					log.msg = "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + log.msg;

					if (!log.path.empty() && log.path[0] != '/') log.path = Settings::program_path + log.path;

					if (log.type < 4) log_to_memory(log.msg, log.type, log.VServ);
					if (log.type > 1 && !log.path.empty()) {
						logMap[log.path] += Utils::str_nocolor(log.msg) + "\n";
					}
				}

				for (std::map<std::string, std::string>::iterator it = logMap.begin(); it != logMap.end(); ++it)
					log_to_file(it->second, it->first);
				logMap.clear();

				Display::update();
			}

		#pragma endregion

		#pragma region Log

			void Log::log(std::string msg, int type, VServer * VServ, std::string path) {
				if (VServ == &(Settings::global)) VServ = NULL;

				if (path.empty()) {
					if (type == BOTH_ACCESS || type == LOCAL_ACCESS) {
						if (VServ) path = VServ->get("access_log");
						else path = Settings::global.get("access_log");
					}
					if (type == BOTH_ERROR || type == LOCAL_ERROR) {
						if (VServ) path = VServ->get("error_log");
						else path = Settings::global.get("error_log");
					}
				}

				Thread::mutex_set(mutex, Thread::MTX_LOCK);
				_logs.push(LogInfo(msg, type, VServ, path));
				Thread::mutex_set(mutex, Thread::MTX_UNLOCK);
			}

		#pragma endregion

	#pragma endregion

	#pragma region Log Rotate

		#pragma region Add

			void Log::add_logrot(std::ofstream & oss, const std::string & log_paths, std::string size, const std::string & user) {
				if (log_paths.empty()) return;
				if (size.empty()) size = Utils::ltos(LOCAL_MAXSIZE);

				oss << "\n" << log_paths << " {\n";
				oss << "\tsize " << size << "\n";
				oss << "\trotate 7\n";
				oss << "\tmissingok\n";
				oss << "\tnotifempty\n";
				if (!user.empty()) oss << "\tcreate 0640 " << user << " " << user << "\n";
				oss << "}\n";
			}

		#pragma endregion

		#pragma region Create

			int Log::create_logrot(const std::string config_path) {
				const char * user = getenv("USER");
				if (user == NULL) user = "";

				std::ofstream oss(config_path.c_str());
				if (oss.is_open()) {
					add_logrot(oss, Utils::escape_spaces(Settings::global.get("access_log")), Settings::global.get("log_maxsize"), user);
					add_logrot(oss, Utils::escape_spaces(Settings::global.get("error_log")), Settings::global.get("log_maxsize"), user);

					for (std::deque <VServer>::iterator vs_it = Settings::vserver.begin(); vs_it != Settings::vserver.end(); ++vs_it) {
						add_logrot(oss, Utils::escape_spaces(vs_it->get("access_log")), vs_it->get("log_maxsize"), user);
						add_logrot(oss, Utils::escape_spaces(vs_it->get("error_log")), vs_it->get("log_maxsize"), user);

						for (std::deque <Location>::iterator loc_it = vs_it->location.begin(); loc_it != vs_it->location.end(); ++loc_it) {
							add_logrot(oss, Utils::escape_spaces(loc_it->get("access_log")), loc_it->get("log_maxsize"), user);
							add_logrot(oss, Utils::escape_spaces(loc_it->get("error_log")), loc_it->get("log_maxsize"), user);
						}

					} oss.close();

					if (::access(config_path.c_str(), F_OK) == 0) return (0);
				}

				return (1);
			}

		#pragma endregion

		#pragma region Get

			std::string Log::get_logrot_path() {
				const char* common_paths[] = {"/usr/sbin/", "/usr/bin/", "/sbin/", "/bin/"};
				int num_paths = sizeof(common_paths) / sizeof(common_paths[0]);

				for (int i = 0; i < num_paths; ++i) {
					std::string full_path = common_paths[i];
					if (::access((full_path + "logrotate").c_str(), F_OK) == 0 && ::access((full_path + "logrotate").c_str(), X_OK) == 0)
						return (full_path + "logrotate");
				}

				return ("");
			}

		#pragma endregion

		#pragma region Execute

			void Log::exec_logrot(const std::string config_path) {
				std::string logrotate_path = get_logrot_path();

				if (logrotate_path.empty() || create_logrot(config_path)) return;
				std::system((logrotate_path +  " " + config_path).c_str());
				remove(config_path.c_str());
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Thread

	#pragma region Main

		void * Log::main(void * args) { (void) args;
			while (Thread::get_bool(mutex, _terminate) == false) {
				Log::process_logs(); usleep(10000);
			}
			return (NULL);
		}

	#pragma endregion

	#pragma region Start

		void Log::start() {
			Thread::mutex_set(mutex, Thread::MTX_INIT);
			Thread::set_bool(mutex, _terminate, false);
			Thread::thread_set(_thread, Thread::THRD_CREATE, main);
		}

	#pragma endregion

	#pragma region Stop

		void Log::stop() {
			Thread::set_bool(mutex, _terminate, true);
			Thread::thread_set(_thread, Thread::THRD_JOIN);
		}

		void Log::release_mutex() {
			Thread::mutex_set(mutex, Thread::MTX_DESTROY);
		}


	#pragma endregion

#pragma endregion
