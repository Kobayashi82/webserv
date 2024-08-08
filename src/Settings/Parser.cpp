/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 21:30:57 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 23:54:15 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"

#include <unistd.h>																						//	For access()
#include <sys/stat.h>																					//	For stat()

#pragma region Brackets

	int Settings::brackets(std::string & str) {
		int change = 0;
		for (size_t i = 0; i < str.size(); ++i) {
			if (str[i] == '{') {
				str.erase(i, 1); --i;																	//	Delete '{' and adjust index
				bracket_lvl++;
				change += 1;
			} else if (str[i] == '}') {
				str.erase(i, 1); --i;																	//	Delete '}' and adjust index
				bracket_lvl--;
				change -= 1;
			}
		}
		return (change);
	}

#pragma endregion

#pragma region Directives

	#pragma region Path

		int Settings::parse_path(std::string & firstPart, std::string & str, bool isFile = false, bool check_path = false) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] "; struct stat info;

			if (str.empty())										Log::log_error(RD + n_line + "Empty value for '" Y + firstPart + RD "'" NC);
			if (str[0] != '/') str = program_path + str;

			if (isFile && check_path) {
				std::string dir_path = str.substr(0, str.find_last_of('/'));
				if (stat(dir_path.c_str(), &info) != 0) {
					if (errno == ENOENT)							Log::log_error(RD + n_line + "The path '" Y + dir_path + RD "' does not exist" NC);
					else if (errno == EACCES)						Log::log_error(RD + n_line + "No permission to access '" Y + dir_path + RD "'" NC);
					else											Log::log_error(RD + n_line + "Cannot access '" Y + dir_path + RD "'" NC);
					return (1);
				} else {
					if (!(info.st_mode & S_IFDIR)) {				Log::log_error(RD + n_line + "'" Y + dir_path + RD "' is not a valid directory" NC); return (1); }
					else if (access(dir_path.c_str(), W_OK) != 0) {	Log::log_error(RD + n_line + "No write permission for '" Y + dir_path + RD "'" NC); return (1); }
					else if (access(str.c_str(), F_OK) == 0
						&& access(str.c_str(), W_OK) != 0) {		Log::log_error(RD + n_line + "No write permission for '" Y + str + RD "'" NC); return (1); }
				}
			} else if (stat(str.c_str(), &info) != 0) {
				if (errno == ENOENT)								Log::log_error(RD + n_line + "The path '" Y + str + RD "' does not exist" NC);
				else if (errno == EACCES)							Log::log_error(RD + n_line + "No permission to access '" Y + str + RD "'" NC);
				else 												Log::log_error(RD + n_line + "Cannot access '" Y + str + RD "'" NC);
				return (1);
			} else {
				if (isFile && !(info.st_mode & S_IFREG)) { 			Log::log_error(RD + n_line + "'" Y + str + RD "' is not a valid file" NC); return (1); return (1); }
				else if (!isFile && !(info.st_mode & S_IFDIR)) { 	Log::log_error(RD + n_line + "'" Y + str + RD "' is not a valid directory" NC); return (1); }
			}
			return (0);
		}

	#pragma endregion

	#pragma region Body Size

		int Settings::parse_body_size(std::string & str) {
			long multiplier = 1; std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";

			if (str.empty()) { Log::log_error(RD + n_line + "Empty value for '" Y "client_max_body_size" RD "'" NC); return (1); }
			if (str.size() > 1 && !std::isdigit(str[str.size() - 1]) && !std::isdigit(str[str.size() - 2]) && str[str.size() - 1] == 'B') str.erase(str.size() - 1);
			if (!std::isdigit(str[str.size() - 1])) {
				switch (str[str.size() - 1]) {
					case 'K': multiplier = 1024; break;
					case 'M': multiplier = 1024 * 1024; break;
					case 'G': multiplier = 1024 * 1024 * 1024; break;
					case 'B' : break;
					default : { Log::log_error(RD + n_line + "Invalid value for '" Y "client_max_body_size" RD "'" NC); return (1); }
				} str.erase(str.size() - 1);
			}

			long number; if (Utils::stol(str, number) || (str = Utils::ltos(number * multiplier)) == "") { Log::log_error(RD + n_line + "Invalid value for '" Y "client_max_body_size" RD "'" NC); return (1); }
			if (number < 1) { Log::log_error(RD + n_line + "Value for '" Y "client_max_body_size" RD "' cannot be lower than 1 byte" NC); return (1); }
			if (number > 1024 * 1024 * 1024) { Log::log_error(RD + n_line + "Value for '" Y "client_max_body_size" RD "' cannot be greater than 1GB" NC); return (1); }
			return (0);
		}

	#pragma endregion

	#pragma region Error Codes

		int Settings::parse_errors(const std::string & firstPart, const std::string & secondPart) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart);
			std::vector<std::string> errors;
			std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { Log::log_error(RD + n_line + "Empty value for '" Y "error_page" RD "'" NC); return (1); }
			std::string filePath = errors.back(); errors.pop_back();
			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (Utils::stol(*it, code) || (error_codes.find(code) == error_codes.end())) {
					Log::log_error(RD + n_line + "Invalid error number '" Y +  *it + RD "' for '" Y "error_page" RD "'" NC); BadConfig = true;
				}
				add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

		int Settings::parse_errors(const std::string & firstPart, const std::string & secondPart, VServer & VServ) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart);
			std::vector<std::string> errors;
			std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { Log::log_error(RD + n_line + "Empty value for '" Y "error_page" RD "'" NC); return (1); }
			std::string filePath = errors.back(); errors.pop_back();

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (Utils::stol(*it, code) || (error_codes.find(code) == error_codes.end())) {
					Log::log_error(RD + n_line + "Invalid error number '" Y +  *it + RD "' for '" Y "error_page" RD "'" NC); BadConfig = true;
				}
				VServ.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

		int Settings::parse_errors(const std::string & firstPart, const std::string & secondPart, Location & Loc) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart);
			std::vector<std::string> errors;
			std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { Log::log_error(RD + n_line + "Empty value for '" Y "error_page" RD "'" NC); return (1); }
			std::string filePath = errors.back(); errors.pop_back();

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (Utils::stol(*it, code) || (error_codes.find(code) == error_codes.end())) {
					Log::log_error(RD + n_line + "Invalid error number '" Y +  *it + RD "' for '" Y "error_page" RD "'" NC); BadConfig = true;
				}
				Loc.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

	#pragma endregion

	#pragma region Autoindex

		int Settings::parse_autoindex(std::string & str) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";

			if (str.empty()) { Log::log_error(RD + n_line + "Empty value for " Y "'autoindex'" NC); return (1); }
			if (str != "on" && str != "off") { Log::log_error(RD + n_line + "Invalid value for " Y "'autoindex'" NC); return (1); }
				
			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region Config File

	#pragma region Location

		int Settings::parser_location(std::ifstream & infile, std::string & line, VServer & VServ) {
			bool inLimit = false; Location Loc; int current_bracket = bracket_lvl; std::string tmp_line; std::string orig_line = line;
			do {
				line_count++; tmp_line = line;
				Utils::trim(line); if (line.empty()) {
					VServ.config.push_back(tmp_line);
					config.push_back(tmp_line);
					continue;
				}
				std::string firstPart, secondPart; std::istringstream stream(line);

				stream >> firstPart; Utils::trim(firstPart); Utils::toLower(firstPart);

				std::getline(stream, secondPart); Utils::trim(secondPart);

				if (tmp_line != orig_line) { VServ.config.push_back(tmp_line); config.push_back(tmp_line); }
				if (!firstPart.empty() && firstPart[firstPart.size() - 1] == ';') firstPart.erase(firstPart.size() - 1);
				if (!secondPart.empty() && secondPart[secondPart.size() - 1] != '{' && secondPart[secondPart.size() - 1] != '}' && secondPart[secondPart.size() - 1] != ';' && firstPart != "{" && firstPart != "}" && firstPart != "location") return (0);
				if (!secondPart.empty() && secondPart[secondPart.size() - 1] == ';') secondPart.erase(secondPart.size() - 1);

				if (brackets(firstPart) + brackets(secondPart) < 0) {
					if (inLimit) inLimit = false;
					if (bracket_lvl < 0) return (0);
					if (bracket_lvl <= current_bracket) { VServ.add(Loc); break; }
				}

				if (firstPart.empty()) continue;
				if (firstPart == "limit_except") inLimit = true;

				if (firstPart == "access_log" || firstPart == "error_log") parse_path(firstPart, secondPart, true, true);
				if (firstPart == "root" && parse_path(firstPart, secondPart, false, false))	BadConfig = true;
				if (firstPart == "client_max_body_size" && parse_body_size(secondPart))		BadConfig = true;
				if (firstPart == "autoindex" && parse_autoindex(secondPart))				BadConfig = true;
				if (firstPart == "error_page") parse_errors(firstPart, secondPart, Loc);
				if (firstPart != "error_page") Loc.add(firstPart, secondPart);
					
			} while (getline(infile, line));
			return (0);
		}

	#pragma endregion

	#pragma region VServer

		int Settings::parser_vserver(std::ifstream & infile, std::string & line) {
			VServer VServ; int current_bracket = bracket_lvl; std::string tmp_line; std::string orig_line = line;
			do {
				tmp_line = line;
				Utils::trim(line); if (line.empty()) {
					VServ.config.push_back(tmp_line);
					config.push_back(tmp_line);
					line_count++; continue;
				}
				std::string firstPart, secondPart; std::istringstream stream(line);
					
				stream >> firstPart; Utils::trim(firstPart); Utils::toLower(firstPart);

				if (firstPart == "location") {
					VServ.config.push_back(tmp_line); config.push_back(tmp_line);
					parser_location(infile, line, VServ);
					firstPart = "";
				}
					
				if (!firstPart.empty()) {
					line_count++;
					std::getline(stream, secondPart); Utils::trim(secondPart);
					if (!firstPart.empty() && firstPart[firstPart.size() - 1] == ';') firstPart.erase(firstPart.size() - 1);
					if (!secondPart.empty() && secondPart[secondPart.size() - 1] != '{' && secondPart[secondPart.size() - 1] != '}' && secondPart[secondPart.size() - 1] != ';' && firstPart != "{" && firstPart != "}" && firstPart != "location") return (0);
					if (!secondPart.empty() && secondPart[secondPart.size() - 1] == ';') secondPart.erase(secondPart.size() - 1);
				}

				if (brackets(firstPart) + brackets(secondPart) < 0) {
					if (bracket_lvl < 0) return (0);
					if (bracket_lvl <= current_bracket) { config.push_back(tmp_line); add(VServ); break; }
				}

				if (!firstPart.empty() && tmp_line != orig_line) { VServ.config.push_back(tmp_line); config.push_back(tmp_line); }
				if (firstPart.empty()) continue;

				if (firstPart == "access_log" || firstPart == "error_log") parse_path(firstPart, secondPart, true, true);
				if (firstPart == "root" && parse_path(firstPart, secondPart, false, false))	BadConfig = true;
				if (firstPart == "client_max_body_size" && parse_body_size(secondPart))		BadConfig = true;
				if (firstPart == "autoindex" && parse_autoindex(secondPart))				BadConfig = true;
				if (firstPart == "error_page") parse_errors(firstPart, secondPart, VServ);
				if (firstPart != "server" && firstPart == "error_page") VServ.add(firstPart, secondPart);

			} while (getline(infile, line));
			return (0);
		}

	#pragma endregion

	#pragma region Global

		int Settings::parse_global(std::ifstream & infile, std::string & line) {
			std::string firstPart, secondPart;
			config.push_back(line);
			Utils::trim(line); if (line.empty()) { line_count++; return (0); }
			std::istringstream stream(line);

			stream >> firstPart; Utils::trim(firstPart); Utils::toLower(firstPart);

			if (firstPart == "server") {
				parser_vserver(infile, line);
				firstPart = "";
			}

			if (!firstPart.empty()) {
				line_count++;
				std::getline(stream, secondPart); Utils::trim(secondPart);
				if (!firstPart.empty() && firstPart[firstPart.size() - 1] == ';') firstPart.erase(firstPart.size() - 1);
				if (!secondPart.empty() && secondPart[secondPart.size() - 1] != '{' && secondPart[secondPart.size() - 1] != '}' && secondPart[secondPart.size() - 1] != ';' && firstPart != "{" && firstPart != "}" && firstPart != "server" && firstPart != "location") return (0);
				if (!secondPart.empty() && secondPart[secondPart.size() - 1] == ';') secondPart.erase(secondPart.size() - 1);
			}

			brackets(firstPart); brackets(secondPart);

			if (firstPart.empty()) return (0);

			if (firstPart == "access_log" || firstPart == "error_log") parse_path(firstPart, secondPart, true, true);
			if (firstPart == "root" && parse_path(firstPart, secondPart, false, false))	BadConfig = true;
			if (firstPart == "client_max_body_size" && parse_body_size(secondPart))		BadConfig = true;
			if (firstPart == "autoindex" && parse_autoindex(secondPart))				BadConfig = true;
			if (firstPart == "error_page") parse_errors(firstPart, secondPart);
			if (firstPart != "error_page") add(firstPart, secondPart);

			return (0);
		}

	#pragma endregion

#pragma endregion
