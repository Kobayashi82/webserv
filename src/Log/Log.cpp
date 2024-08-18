/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/18 14:37:02 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"
#include "Log.hpp"

#pragma region Variables

	size_t	Log::_log_days = 30;

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

		static void log_to_file(const std::string & str, std::string path = "") {
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

		void Log::log_access(std::string str, VServer * VServ) {
			if (str.empty() || Settings::check_only) return ;
			str = "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + str;
			Settings::global.log.both_add(str); Settings::global.log.access_add(str);
			if (VServ) { VServ->log.both_add(str); VServ->log.access_add(str); }
			if (VServ && VServ->get("access_log") != "") log_to_file(str, VServ->get("access_log"));
			else log_to_file(str, Settings::global.get("access_log"));
			Display::Output();
		}

	#pragma endregion

	#pragma region Log Error

		void Log::log_error(std::string str, VServer * VServ) {
			if (str.empty()) return ;
			if (!Settings::check_only && !Display::RawModeDisabled && !Display::ForceRawModeDisabled) str = "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + str;
			else str = "\t" + str + NC;
			Settings::global.log.both_add(str); Settings::global.log.error_add(str);
			if (VServ) { VServ->log.both_add(str); VServ->log.error_add(str); }
			if (VServ && VServ->get("error_log") != "") log_to_file(str, VServ->get("error_log"));
			else log_to_file(str, Settings::global.get("error_log"));
			Display::Output();
		}

	#pragma endregion

#pragma endregion
 