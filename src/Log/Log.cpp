/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/02 22:56:33 by vzurera-         ###   ########.fr       */
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
	long						Log::LOCAL_ROTATESIZE = 1 * 1024 * 1024;									//	Maximum size of the log before rotation	(default to 1 MB | 0 MB = no rotation | Max 100 MB)
	int							Log::LOCAL_ROTATE = 7;														//	Number of rotations files 				(default to 7 | 0 = no rotations files | Max 100)

	#pragma region LogInfo

		#pragma region Constructors

			Log::LogInfo::LogInfo(std::string _msg, int _type, VServer * _VServ, std::string _path) : msg(_msg), type(_type), VServ(_VServ), path(_path) {}

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
				if (msg.empty()) return;

				if (!Display::background && (Settings::check_only || !Display::isRawMode() || Display::ForceRawModeDisabled)) {
					if (msg == "---") std::cout << " " << Y "-----------------------------------------------------------------------------------------" << NC "\n";
					else			  std::cout << " " << msg << NC "\n";
					return;
				}

				Thread::mutex_set(mutex, Thread::MTX_LOCK);

					if (type == VSERV_ACCESS) {
						if (VServ) { VServ->log.both_add(msg); VServ->log.access_add(msg); }
					} else if (type == VSERV_ERROR) {
						if (VServ) { VServ->log.both_add(msg); VServ->log.error_add(msg); }
					} else if (type == GLOBAL_ACCESS) {
						Settings::global.log.both_add(msg); Settings::global.log.access_add(msg);
					} else if (type == GLOBAL_ERROR) {
						Settings::global.log.both_add(msg); Settings::global.log.error_add(msg);
					} else if (type == MEM_ACCESS || type == BOTH_ACCESS) {
						Settings::global.log.both_add(msg); Settings::global.log.access_add(msg);
						if (VServ) { VServ->log.both_add(msg); VServ->log.access_add(msg); }
					} else if (type == MEM_ERROR || type == BOTH_ERROR) {
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
				std::ostringstream oss;

				while (!logs.empty()) {
					Log::LogInfo log = logs.front(); logs.pop();
					
					Utils::trim(log.msg); if (log.msg.empty()) continue;

					bool isTRF = false;
					if (log.msg.size() > 3 && log.msg.substr(0, 3) == "TRF") {
						if (log.type < LOCAL_ACCESS) { isTRF = true; log_request(log.msg, log.VServ, false); }
					} else {
						Thread::mutex_set(Log::mutex, Thread::MTX_LOCK);
						if (log.msg != "---" && !(Settings::check_only && Settings::loaded == false))
							log.msg = BLUE600 "[" LIME600 + Settings::timer.current_date() + " " EMERALD400 + Settings::timer.current_time() + BLUE600 "]  " NC + log.msg;
						Thread::mutex_set(Log::mutex, Thread::MTX_UNLOCK);
					}

					if (!log.path.empty() && log.path[0] != '/') log.path = Settings::program_path + log.path;

					if (log.type < LOCAL_ACCESS) { if (isTRF == false) log_to_memory(log.msg, log.type, log.VServ); }
					if (log.type > GLOBAL_ERROR && !log.path.empty()) {
						if (log.msg.size() > 3 && log.msg.substr(0, 3) == "TRF") log.msg = log_request(log.msg, log.VServ, true);
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

			#pragma region Log Request

				std::string Log::log_request(std::string log_msg, VServer * VServ, bool isLocal) {
					if (log_msg.empty()) return ("");
					try {
						long		code;
						size_t		bytes;

						std::string	method;
						std::string	re_path;
						std::string	scode;
						std::string	sbytes;
						std::string	time;
						std::string	ip;
						
						std::istringstream iss(log_msg);
						std::getline(iss, log_msg, '|');
						std::getline(iss, method, '|');
						std::getline(iss, re_path, '|');
						std::getline(iss, scode, '|');
						std::getline(iss, sbytes, '|');
						std::getline(iss, time, '|');
						std::getline(iss, ip, '|');

						Utils::stol(scode, code);
						Utils::stol(sbytes, bytes);

						std::string method_color;
						if (method == "GET") method_color = AMBER500;
						else if (method == "HEAD") method_color = GREEN600;
						else if (method == "POST") method_color = FUCHSIA600;
						else if (method == "PUT") method_color = TEAL400;
						else if (method == "PATCH") method_color = TEAL400;
						else if (method == "DELETE") method_color = RED400;
						else  method_color = RED800;

						std::string space = "";
						int method_size = 12;
						if (method.size() <= 12) method_size = method.size();
						if (method_size == 12) space = " ";
						std::string msg1 = UN BLUE400 + ip + NC + std::string("                ").substr(ip.size()) + method_color + method + std::string("            ").substr(method_size) + space + BR + re_path + NC;

						std::string s_bytes = Utils::formatSize(bytes);

						std::string code_color;
						if (code >= 100 && code < 300) code_color = GREEN600;
						if (code >= 300 && code < 400) code_color = ORANGE400;
						if (code >= 400 && code < 600) code_color = RED400;

						std::string msg2 = BLUE800 "Transfered:     " AMBER200 + s_bytes + std::string("           ").substr(s_bytes.size()) + C " in " SKY700 + time + " ms " C "with code " + code_color + Utils::ltos(code) + C " (" + code_color + Settings::error_codes[code] + C ")" NC;

						Thread::mutex_set(mutex, Thread::MTX_LOCK);
							std::string logtime = BLUE600 "[" LIME600 + Settings::timer.current_date() + " " EMERALD400 + Settings::timer.current_time() + BLUE600 "]  " NC;
						Thread::mutex_set(mutex, Thread::MTX_UNLOCK);

						if (isLocal) {
							method_size = 10;
							space = "";
							if (method.size() <= 10) method_size = method.size();
							if (method_size == 10) space = " ";
							std::string msg3 = ip + std::string("                ").substr(ip.size()) + method_color + method + std::string("          ").substr(method_size) + space;
							std::string msg4 = scode + "    " + s_bytes + std::string("           ").substr(s_bytes.size()) + time + " ms" + std::string("         ").substr(time.size()) + re_path;

							return (logtime + msg3 + msg4);
						}

						msg1 = logtime + msg1;
						msg2 = "                       " + msg2;
						
						if (!Display::background && (Settings::check_only || !Display::isRawMode() || Display::ForceRawModeDisabled)) {
							std::cout << " " << msg1 << "\n " << msg2 << "\n " <<  Y "-----------------------------------------------------------------------------------------" << NC "\n";
						} else {

							Thread::mutex_set(mutex, Thread::MTX_LOCK);
							
								Settings::global.log.both_add(msg1); Settings::global.log.access_add(msg1);
								if (VServ) { VServ->log.both_add(msg1); VServ->log.access_add(msg1); }
								Settings::global.log.both_add(msg2); Settings::global.log.access_add(msg2);
								if (VServ) { VServ->log.both_add(msg2); VServ->log.access_add(msg2); }
								Settings::global.log.both_add("---"); Settings::global.log.access_add("---");
								if (VServ) { VServ->log.both_add("---"); VServ->log.access_add("---"); }

							Thread::mutex_set(mutex, Thread::MTX_UNLOCK);

						}

					} catch (...) {}
					return ("");
				}

		#pragma endregion

			#pragma region Log

				void Log::log(std::string msg, int type, VServer * VServ, std::vector<std::pair<std::string, std::string> > * data) {
					std::string path;

					if (VServ == &(Settings::global)) VServ = NULL;

					if (!VServ) {
						if (type == BOTH_ACCESS || type == LOCAL_ACCESS)	path = Settings::data_get(Settings::global.data, "access_log");
						if (type == BOTH_ERROR  || type == LOCAL_ERROR )	path = Settings::data_get(Settings::global.data, "error_log" );

						Thread::mutex_set(mutex, Thread::MTX_LOCK);

							_logs.push(LogInfo(msg, type, VServ, path));

						Thread::mutex_set(mutex, Thread::MTX_UNLOCK);
					} else {
						if (!data) data = &VServ->data;

						if (type == BOTH_ACCESS || type == LOCAL_ACCESS)	path = Settings::data_get(data, "access_log");
						if (type == BOTH_ERROR  || type == LOCAL_ERROR )	path = Settings::data_get(data, "error_log" );

						if (path.empty()) {
							data = &VServ->data;
							if (type == BOTH_ACCESS || type == LOCAL_ACCESS)	path = Settings::data_get(data, "access_log");
							if (type == BOTH_ERROR  || type == LOCAL_ERROR )	path = Settings::data_get(data, "error_log" );
						}

						if (path.empty()) {
							if (type == BOTH_ACCESS || type == LOCAL_ACCESS)	path = Settings::data_get(Settings::global.data, "access_log");
							if (type == BOTH_ERROR  || type == LOCAL_ERROR )	path = Settings::data_get(Settings::global.data, "error_log" );
						}

						Thread::mutex_set(mutex, Thread::MTX_LOCK);

							_logs.push(LogInfo(msg, type, VServ, path));

						Thread::mutex_set(mutex, Thread::MTX_UNLOCK);
					}
				}

			#pragma endregion

		#pragma endregion

	#pragma endregion

	#pragma region Rotate

		#pragma region Add

			void Log::add_logrot(std::ofstream & oss, const std::string & log_paths, std::string size, std::string rotate, const std::string & user) {
				if (log_paths.empty()) return;
				if (size.empty()) size = Utils::ltos(LOCAL_ROTATESIZE);
				if (rotate.empty()) rotate = Utils::ltos(LOCAL_ROTATE);

				oss << "\n" << log_paths << " {\n";
				oss << "\tsize " << size << "\n";
				oss << "\trotate " << rotate << "\n";
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
					add_logrot(oss, Utils::escape_spaces(Settings::global.get("access_log")), Settings::global.get("log_maxsize"), Settings::global.get("log_rotate"), user);
					add_logrot(oss, Utils::escape_spaces(Settings::global.get("error_log")), Settings::global.get("log_maxsize"), Settings::global.get("log_rotate"), user);

					for (std::deque <VServer>::iterator vs_it = Settings::vserver.begin(); vs_it != Settings::vserver.end(); ++vs_it) {
						add_logrot(oss, Utils::escape_spaces(vs_it->get("access_log")), vs_it->get("log_maxsize"), vs_it->get("log_rotate"), user);
						add_logrot(oss, Utils::escape_spaces(vs_it->get("error_log")), vs_it->get("log_maxsize"), vs_it->get("log_rotate"), user);

						for (std::deque <Location>::iterator loc_it = vs_it->location.begin(); loc_it != vs_it->location.end(); ++loc_it) {
							add_logrot(oss, Utils::escape_spaces(loc_it->get("access_log")), loc_it->get("log_maxsize"), loc_it->get("log_rotate"), user);
							add_logrot(oss, Utils::escape_spaces(loc_it->get("error_log")), loc_it->get("log_maxsize"), loc_it->get("log_rotate"), user);
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
				return;																					//	Disabled because we dont have admin rights
				std::string logrotate_path = get_logrot_path();

				if (logrotate_path.empty() || create_logrot(config_path)) return;
				std::cout << " - " << logrotate_path << "\n";
				std::system((logrotate_path +  " '" + config_path + "' > /dev/null 2>&1").c_str());
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
