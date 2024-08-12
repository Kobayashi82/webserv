/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/12 18:57:36 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Log.hpp"

#pragma region Variables

	static size_t				_maxSize = 200;
	std::deque <std::string>	Log::access;
	std::deque <std::string>	Log::error;
	std::deque <std::string>	Log::both;

#pragma endregion

#pragma region Memory Logs

	#pragma region Add

		static void access_add(const std::string & str, VServer * VServ = NULL) {
			if (VServ) {
				if (VServ->access.size() == _maxSize) VServ->access.pop_front();
				VServ->access.push_back(str);
			} else {
				if (Log::access.size() == _maxSize) Log::access.pop_front();
				Log::access.push_back(str);
			}
		}

		static void error_add(const std::string & str, VServer * VServ = NULL) {
			if (VServ) {
				if (VServ->error.size() == _maxSize) VServ->error.pop_front();
				VServ->error.push_back(str);
			} else {
				if (Log::error.size() == _maxSize) Log::error.pop_front();
				Log::error.push_back(str);
			}
		}

		static void both_add(const std::string & str, VServer * VServ = NULL) {
			if (VServ) {
				if (VServ->both.size() == _maxSize) VServ->both.pop_front();
				VServ->both.push_back(str);
			} else {
				if (Log::both.size() == _maxSize) Log::both.pop_front();
				Log::both.push_back(str);
			}
		}

	#pragma endregion

	#pragma region Print

		std::string Log::get_access(VServer * VServ, size_t rows) {
			std::ostringstream stream;
			if (VServ) {
				for (size_t i = 0; i < rows && i < VServ->access.size(); ++i) {
					stream << VServ->access[i];
					if (i + 1 != VServ->access.size()) stream << NC << std::endl;
				}
			} else {
				for (size_t i = 0; i < rows && i < access.size(); ++i) {
					stream << access[i];
					if (i + 1 != access.size()) stream << NC << std::endl;
				}
			}
			return (stream.str());
		}

		std::string Log::get_error(VServer * VServ, size_t rows) {
			std::ostringstream stream;
			if (VServ) {
				for (size_t i = 0; i < rows && i < VServ->error.size(); ++i) {
					stream << VServ->error[i];
					if (i + 1 != VServ->error.size()) stream << NC << std::endl;
				}
			} else {
				for (size_t i = 0; i < rows && i < error.size(); ++i) {
					stream << error[i];
					if (i + 1 != error.size()) stream << NC << std::endl;
				}
			}
			return (stream.str());
		}

		std::string Log::get_both(VServer * VServ, size_t rows) {
			std::ostringstream stream;
			if (VServ) {
				for (size_t i = 0; i < rows && i < VServ->both.size(); ++i) {
					stream << VServ->both[i];
					if (i + 1 != VServ->both.size()) stream << NC << std::endl;
				}
			} else {
				for (size_t i = 0; i < rows && i < both.size(); ++i) {
					stream << both[i];
					if (i + 1 != both.size()) stream << NC << std::endl;
				}
			}
			return (stream.str());
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
			both_add(str); access_add(str);
			if (VServ) { both_add(str, VServ); access_add(str, VServ); }
			if (VServ && VServ->get("access_log") != "") log_to_file(str, VServ->get("access_log"));
			else log_to_file(str, Settings::get("access_log"));
			Display::Output();
		}

	#pragma endregion

	#pragma region Log Error

		void Log::log_error(std::string str, VServer * VServ) {
			if (str.empty()) return ;
			if (!(Settings::check_only || Display::RawModeDisabled || Display::ForceRawModeDisabled) && !Settings::loaded_ok) str = "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + str;
			else str = "\t" + str + NC;
			both_add(str); error_add(str);
			if (VServ) { both_add(str, VServ); error_add(str, VServ); }
			if (VServ && VServ->get("error_log") != "") log_to_file(str, VServ->get("error_log"));
			else log_to_file(str, Settings::get("error_log"));
			Display::Output();
		}

	#pragma endregion

#pragma endregion
 