/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/15 19:06:34 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"
#include "Log.hpp"

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
 