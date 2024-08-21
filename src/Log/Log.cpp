/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/21 22:22:13 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"
#include "Display.hpp"
#include "Mutex.hpp"
#include "Log.hpp"

#pragma region Variables

	size_t									Log::_log_days = 30;
	std::queue <Log::LogInfo>				Log::_logs;
	std::map <std::string, std::ofstream *>	Log::_fileStreams;

#pragma endregion


#pragma region LogInfo

	#pragma region Constructors

		Log::LogInfo::LogInfo(std::string & _msg, int _type, VServer * _VServ) : msg(_msg), type(_type), VServ(_VServ) {}

	#pragma endregion

	#pragma region Overloads

		Log::LogInfo & Log::LogInfo::operator=(const LogInfo & rhs) {
			if (this != &rhs) { msg = rhs.msg; type = rhs.type; VServ = rhs.VServ; }
			return (*this);
		}

		bool Log::LogInfo::operator==(const LogInfo & rhs) {
			return (msg == rhs.msg && type == rhs.type && VServ == rhs.VServ);
		}

	#pragma endregion

#pragma endregion



#pragma region Constructors

	Log::Log() : _maxSize(200) {}
    Log::Log(const Log & src) { *this = src; }
	Log::~Log() { clear(); }

#pragma endregion

#pragma region Overloads

	Log & Log::operator=(const Log & rhs) { if (this != &rhs) { _maxSize = rhs._maxSize; access = rhs.access; error = rhs.error; both = rhs.both; } return (*this); }

#pragma endregion

#pragma region Memory Logs

	#pragma region Add

		void Log::access_add(const std::string & str) {
			if (access.size() == _maxSize) access.pop_front();
				access.push_back(str);
		}

		void Log::error_add(const std::string & str) {
			if (error.size() == _maxSize) error.pop_front();
				error.push_back(str);
		}

		void Log::both_add(const std::string & str) {
			if (both.size() == _maxSize) both.pop_front();
				both.push_back(str);
		}

	#pragma endregion

	#pragma region Clear

		void Log::clear() { access.clear(); error.clear(); both.clear(); }

	#pragma endregion

#pragma endregion

#pragma region Local Logs

	#pragma region Check Logs

		#pragma region Valid Date

			static bool valid_date(const std::string & date, int log_days) {
				struct tm dateTm = {}; int day, month, year;
				if (sscanf(date.c_str(), "%d/%d/%d", &day, &month, &year) != 3) return (false);
				 if (day < 1 || day > 31 || month < 1 || month > 12 || year < 1900) return (false);
				dateTm.tm_mday = day; dateTm.tm_mon = month - 1; dateTm.tm_year = year - 1900;

				time_t timeDate = mktime(&dateTm);
				if (timeDate == -1) return (false);
				
				time_t timeCurr = time(NULL);
				double diff = difftime(timeCurr, timeDate);
				double secondsMax = (log_days + 1) * 24 * 60 * 60;
				return (diff <= secondsMax);
			}

		#pragma endregion

		#pragma region Clear Log

			static void clear_log(const std::string & log_file, int log_days) {
				std::string line;
				std::ifstream orig_file(log_file.c_str()); if (!orig_file.is_open()) return;
				std::ofstream temp_file((log_file + "_temp").c_str()); if (!temp_file.is_open()) { orig_file.close(); return; }

				while (std::getline(orig_file, line))
					if (valid_date(line.substr(1, 11), log_days)) temp_file << line << std::endl;
				
				orig_file.close(); temp_file.close();

				std::remove(log_file.c_str());
				std::rename((log_file + "_temp").c_str(), log_file.c_str());
			}

		#pragma endregion

		#pragma region Check Logs

			void Log::check_logs() {
				return;
				if (!Settings::global.data.empty()) {
					long log_days; if (Utils::stol(Settings::global.get("log_days"), log_days)) log_days = _log_days;
					std::string access = Settings::global.get("access_log");
					std::string error = Settings::global.get("error_log");
					if (!access.empty()) clear_log(access, log_days);
					if (!error.empty()) clear_log(error, log_days);
				}

				for (std::deque <VServer>::iterator server = Settings::vserver.begin(); server != Settings::vserver.end(); ++server) {
					if (!server->data.empty()) {
						long log_days; if (Utils::stol(server->get("log_days"), log_days)) log_days = _log_days;
						std::string access = server->get("access_log");
						std::string error = server->get("error_log");
						if (!access.empty()) clear_log(access, log_days);
						if (!error.empty()) clear_log(error, log_days);
					}

					for (std::deque <Location>::iterator location = server->location.begin(); location != server->location.end(); ++location) {
						if (!location->data.empty()) {
							long log_days; if (Utils::stol(location->get("log_days"), log_days)) log_days = _log_days;
							std::string access = location->get("access_log");
							std::string error = location->get("error_log");
							if (!access.empty()) clear_log(access, log_days);
							if (!error.empty()) clear_log(error, log_days);
						}
					}
				}
			}

		#pragma endregion

	#pragma endregion

	#pragma region Log to File

		void log_to_file(const std::string & str, std::string path = "") {
			if (str.empty()) return;
			if (Settings::check_only || Display::RawModeDisabled || Display::ForceRawModeDisabled) std::cout << " " << str << std::endl;
			else {
				if (path != "" && path[0] != '/') path = Settings::program_path + path;
				std::ofstream outfile;
				outfile.open(path.c_str(), std::ios_base::app);
				if (outfile.is_open()) {
					outfile << Utils::str_nocolor(str) << std::endl;
					outfile.close();
				}
			}
		}

	#pragma endregion

	#pragma region Log Access

		void Log::log_access(std::string msg, int type, VServer * VServ) {
			if (msg.empty()) return ;
			if (!Settings::check_only && !Display::RawModeDisabled && !Display::ForceRawModeDisabled)
				msg = "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + msg;
			else msg = "\t" + msg + NC;
			if (type == MEM_ACCESS || type == BOTH_ACCESS) {
				Settings::global.log.both_add(msg); Settings::global.log.access_add(msg);
				if (VServ) { VServ->log.both_add(msg); VServ->log.access_add(msg); }
			}

			// if (type == LOCAL_ACCESS || type == BOTH_ACCESS) {
			// 	if (VServ && VServ->get("access_log") != "") log_to_file(msg, VServ->get("access_log"));
			// 	else log_to_file(msg, Settings::global.get("access_log"));
			// }
			//Display::Output();
		}

	#pragma endregion

	#pragma region Log Error

		void Log::log_error(std::string msg, int type, VServer * VServ) {
			if (msg.empty()) return ;
			if (!Settings::check_only && !Display::RawModeDisabled && !Display::ForceRawModeDisabled)
				msg = "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + msg;
			else msg = "\t" + msg + NC;
			if (type == MEM_ERROR || type == BOTH_ERROR) {
				Settings::global.log.both_add(msg); Settings::global.log.error_add(msg);
				if (VServ) { VServ->log.both_add(msg); VServ->log.error_add(msg); }
			}
			// if (type == LOCAL_ERROR || type == BOTH_ERROR) {
			// 	if (VServ && VServ->get("error_log") != "") log_to_file(msg, VServ->get("error_log"));
			// 	else log_to_file(msg, Settings::global.get("error_log"));
			// }
			//Display::Output();
		}

	#pragma endregion

#pragma endregion

void Log::close_fileStreams() {
    for (std::map <std::string, std::ofstream *>::iterator it = _fileStreams.begin(); it != _fileStreams.end(); ++it) {
        if (it->second) {
			it->second->close();
			delete it->second;
		}
	}
    _fileStreams.clear();
}

std::ofstream * Log::get_fileStream(const std::string & path) {
    std::map <std::string, std::ofstream *>::iterator it = _fileStreams.find(path);
    if (it == _fileStreams.end()) {
		std::ofstream * outfile = new std::ofstream(path.c_str(), std::ios_base::app);
        if (outfile->is_open()) {
            _fileStreams[path] = outfile;
			return (outfile);
		} else {
			delete outfile;
			return (NULL);
		}
    }
    return (it->second);
}

	#pragma region Log to File

		void Log::log_to_file2(const std::string & msg, std::string path) {
			if (msg.empty() || path.empty()) return;
			if (Settings::check_only || Display::RawModeDisabled || Display::ForceRawModeDisabled) std::cout << " " << msg << std::endl;
			else {
				std::ofstream * ofs = get_fileStream(path);
				if (ofs) *ofs << msg; ofs->flush();

				// if (path != "" && path[0] != '/') path = Settings::program_path + path;
				// std::ofstream outfile;
				// outfile.open(path.c_str(), std::ios_base::app);
				// if (outfile.is_open()) { outfile << msg; outfile.close(); }
			}
		}

	#pragma endregion

 void	Log::log(std::string msg, int type, VServer * VServ) {
	if (VServ == &(Settings::global)) VServ = NULL;
	Mutex::mtx_set(Mutex::MTX_LOCK);
	_logs.push(LogInfo(msg, type, VServ));
	Mutex::mtx_set(Mutex::MTX_UNLOCK);
 }
 
void Log::process_logs() {
	std::queue <Log::LogInfo> logs;

	Mutex::mtx_set(Mutex::MTX_LOCK);
	if (_logs.empty()) { Mutex::mtx_set(Mutex::MTX_UNLOCK); return; }
	std::swap(logs, _logs);
	Mutex::mtx_set(Mutex::MTX_UNLOCK);

	std::map<std::string, std::string> logMap;

	while (!logs.empty()) {
		Log::LogInfo log = logs.front(); logs.pop();
		std::string path = Settings::global.get("access_log");
		if (path != "" && path[0] != '/') path = Settings::program_path + path;
		if (!log.msg.empty() && !path.empty())
			logMap[path] += "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + Utils::str_nocolor(log.msg) + "\n";
		switch (log.type) {
			case MEM_ACCESS: log_access(log.msg, log.type, log.VServ); break;
			case MEM_ERROR: log_error(log.msg, log.type, log.VServ); break;
			case LOCAL_ACCESS: log_access(log.msg, log.type, log.VServ); break;
			case LOCAL_ERROR: log_error(log.msg, log.type, log.VServ); break;
			case BOTH_ACCESS: log_access(log.msg, log.type, log.VServ); break;
			case BOTH_ERROR: log_error(log.msg, log.type, log.VServ); break;
		}
    }
    for (std::map<std::string, std::string>::iterator it = logMap.begin(); it != logMap.end(); ++it)
        log_to_file2(it->second, it->first);
    logMap.clear();
	Display::Output();
}

#pragma endregion
 
//  void	Log::log(std::string msg, std::string path) {
// 	path = Settings::global.get("access_log");
// 	Mutex::mtx_set(Mutex::MTX_LOCK);
// 	_logs.push(LogInfo(msg, path));
// 	Mutex::mtx_set(Mutex::MTX_UNLOCK);
//  }
 
// void Log::process_logs() {
// 	std::queue <Log::LogInfo> logs;

// 	Mutex::mtx_set(Mutex::MTX_LOCK);
// 	if (_logs.empty()) { Mutex::mtx_set(Mutex::MTX_UNLOCK); return; }
// 	std::swap(logs, _logs);
// 	Mutex::mtx_set(Mutex::MTX_UNLOCK);

// 	std::map<std::string, std::string> logMap;

// 	while (!logs.empty()) {
// 	   	Log::LogInfo log = logs.front(); logs.pop();
// 		if (!log.msg.empty() && !log.path.empty()) {
// 			logMap[log.path] += "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + log.msg + "\n";
// 			//log.msg = "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + log.msg;
// 			//log_to_file(msg, log.path);
// 		}
// 	}
//     for (std::map<std::string, std::string>::iterator it = logMap.begin(); it != logMap.end(); ++it)
//         log_to_file(it->second, it->first);
//     logMap.clear();
// 	//std::cout << "FIN\n";
// }