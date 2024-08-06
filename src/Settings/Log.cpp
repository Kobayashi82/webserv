/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/06 01:06:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Log.hpp"

#pragma region Variables

	static size_t				_maxSize = 200;
	std::deque <std::string>	Log::Access;
	std::deque <std::string>	Log::Error;
	std::deque <std::string>	Log::Both;

#pragma endregion

#pragma region Memory Logs

	#pragma region Add

		static void access_add(const std::string & str, VServer * VServ = NULL) {
			if (VServ) {
				if (VServ->access.size() == _maxSize) VServ->access.pop_front();
				VServ->access.push_back(str);
			} else {
				if (Log::Access.size() == _maxSize) Log::Access.pop_front();
				Log::Access.push_back(str);
			}
		}

		static void error_add(const std::string & str, VServer * VServ = NULL) {
			if (VServ) {
				if (VServ->error.size() == _maxSize) VServ->error.pop_front();
				VServ->error.push_back(str);
			} else {
				if (Log::Error.size() == _maxSize) Log::Error.pop_front();
				Log::Error.push_back(str);
			}
		}

		static void both_add(const std::string & str, VServer * VServ = NULL) {
			if (VServ) {
				if (VServ->both.size() == _maxSize) VServ->both.pop_front();
				VServ->both.push_back(str);
			} else {
				if (Log::Both.size() == _maxSize) Log::Both.pop_front();
				Log::Both.push_back(str);
			}
		}

	#pragma endregion

	#pragma region Print

		std::string Log::access(VServer * VServ, size_t rows) {
			std::ostringstream stream;
			if (VServ) {
				for (size_t i = 0; i < rows && i < VServ->access.size(); ++i) {
					stream << VServ->access[i];
					if (i + 1 != VServ->access.size()) stream << NC << std::endl;
				}
			} else {
				for (size_t i = 0; i < rows && i < Access.size(); ++i) {
					stream << Access[i];
					if (i + 1 != Access.size()) stream << NC << std::endl;
				}
			}
			return (stream.str());
		}

		std::string Log::error(VServer * VServ, size_t rows) {
			std::ostringstream stream;
			if (VServ) {
				for (size_t i = 0; i < rows && i < VServ->error.size(); ++i) {
					stream << VServ->error[i];
					if (i + 1 != VServ->error.size()) stream << NC << std::endl;
				}
			} else {
				for (size_t i = 0; i < rows && i < Error.size(); ++i) {
					stream << Error[i];
					if (i + 1 != Error.size()) stream << NC << std::endl;
				}
			}
			return (stream.str());
		}

		std::string Log::both(VServer * VServ, size_t rows) {
			std::ostringstream stream;
			if (VServ) {
				for (size_t i = 0; i < rows && i < VServ->both.size(); ++i) {
					stream << VServ->both[i];
					if (i + 1 != VServ->both.size()) stream << NC << std::endl;
				}
			} else {
				for (size_t i = 0; i < rows && i < Both.size(); ++i) {
					stream << Both[i];
					if (i + 1 != Both.size()) stream << NC << std::endl;
				}
			}
			return (stream.str());
		}

	#pragma endregion

	#pragma region Clear

		void Log::clear() { Access.clear(); Error.clear(); Both.clear(); }

	#pragma endregion

#pragma endregion

#pragma region Local Logs

	#pragma region Log to File

		static void log_to_file(const std::string & str, std::string path = "") {
			if (Settings::check_only) std::cout << str << std::endl;
			else {
				if (path != "" && path[0] != '/') path = Settings::program_path + path;
				Settings::createPath(path);
				std::ofstream outfile;
				outfile.open(path.c_str(), std::ios_base::app);
				if (outfile.is_open()) {
					outfile << str << std::endl;
					outfile.close();
				}
			}
		}

	#pragma endregion

	#pragma region Log Access

		void Log::log_access(std::string str, VServer * VServ) {
			if (str.empty()) return ;
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
			str = "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + str;
			both_add(RD + str); error_add(str);
			if (VServ) { both_add(str, VServ); error_add(str, VServ); }
			if (VServ && VServ->get("error_log") != "") log_to_file(str, VServ->get("error_log"));
			else log_to_file(str, Settings::get("error_log"));
			Display::Output();
		}

	#pragma endregion

#pragma endregion
 