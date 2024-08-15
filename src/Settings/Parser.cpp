/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 21:30:57 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/15 18:36:44 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"

#include <unistd.h>																						//	For access()
#include <sys/stat.h>																					//	For stat()

static int remove_semicolon(std::string & str, int line_count, bool all = false) {
    size_t pos; std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";

	if (str.empty()) return (0);
	while ((pos = str.find("{;")) != std::string::npos) str.erase(pos + 1, 1);
	while ((pos = str.find("};")) != std::string::npos) str.erase(pos + 1, 1);

	if (all) {
		std::string firstPart; std::istringstream stream(str);
		stream >> firstPart; Utils::trim(firstPart); Utils::toLower(firstPart);

		if (str[str.size() - 1] != '{' && str[str.size() - 1] != '}' && str[str.size() - 1] != ';'
		&& firstPart != "http" && firstPart != "server" && firstPart != "location" && firstPart != "limit_except") {
			Log::log_error(RD + n_line + "no termina correctamente"); return (1); }

		while ((pos = str.find(";")) != std::string::npos) str.erase(pos, 1);
	}

	return (0);
}

static int check_multiline(const std::string & str, int line_count) {
    size_t pos; std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";

    if ((pos = str.find(";")) != std::string::npos && (str.find("{") != std::string::npos || str.find("}") != std::string::npos || str.find(";", pos + 1) != std::string::npos)) {
        Log::log_error(RD + n_line + "Found ';{' in the string"); return (1); }

	return (0);
}

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

		int Settings::parse_path(const std::string & firstPart, std::string & str, bool isFile = false, bool check_path = false, bool check_write = false) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] "; struct stat info;

			if (str.empty()) {										Log::log_error(RD + n_line + "Empty value for " Y + firstPart + NC); return (1); }
			if (str[0] != '/') str = program_path + str;

			if (isFile && check_path) {
				std::string dir_path = str.substr(0, str.find_last_of('/'));
				if (stat(dir_path.c_str(), &info) != 0) {
					if (errno == ENOENT)							Log::log_error(RD + n_line + "The " Y + firstPart + RD " path " Y + dir_path + RD " does not exist" NC);
					else if (errno == EACCES)						Log::log_error(RD + n_line + "No permission to access " Y + dir_path + NC);
					else											Log::log_error(RD + n_line + "Cannot access " Y + dir_path + NC);
					return (1);
				} else {
					if (!(info.st_mode & S_IFDIR)) {				Log::log_error(RD + n_line + dir_path + RD " is not a valid directory" NC); return (1); }
					else if (firstPart != "access_log" && firstPart != "error_log" && access(str.c_str(), F_OK) != 0) {
																	Log::log_error(RD + n_line + "The " Y + firstPart + RD " path " Y + str + RD " does not exist" NC); return (1); }
					else if (check_write &&
						access(dir_path.c_str(), W_OK) != 0) {		Log::log_error(RD + n_line + "No write permission for " Y + dir_path + NC); return (1); }
					else if (check_write && access(str.c_str(), F_OK) == 0 &&
						access(str.c_str(), W_OK) != 0) {			Log::log_error(RD + n_line + "No write permission for " Y + str + NC); return (1); }
				}
			} else if (stat(str.c_str(), &info) != 0) {
				if (errno == ENOENT)								Log::log_error(RD + n_line + "The " Y + firstPart + RD " path " Y + str + RD " does not exist" NC);
				else if (errno == EACCES)							Log::log_error(RD + n_line + "No permission to access " Y + str + NC);
				else 												Log::log_error(RD + n_line + "Cannot access " Y + str + NC);
				return (1);
			} else {
				if (isFile && !(info.st_mode & S_IFREG)) { 			Log::log_error(RD + n_line + Y + str + RD " is not a valid file" NC); return (1); return (1); }
				else if (!isFile && !(info.st_mode & S_IFDIR)) { 	Log::log_error(RD + n_line + Y + str + RD " is not a valid directory" NC); return (1); }
				else if (access(str.c_str(), R_OK) != 0) {			Log::log_error(RD + n_line + "No permission to access " Y + str + NC); return (1); }
			}
			return (0);
		}

	#pragma endregion

	#pragma region Body Size

		int Settings::parse_body_size(std::string & str) {
			long multiplier = 1; std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";

			if (str.empty()) { Log::log_error(RD + n_line + "Empty value for " Y "client_max_body_size" NC); return (1); }
			if (str.size() > 1 && !std::isdigit(str[str.size() - 1]) && !std::isdigit(str[str.size() - 2]) && str[str.size() - 1] == 'B') str.erase(str.size() - 1);
			if (!std::isdigit(str[str.size() - 1])) {
				switch (str[str.size() - 1]) {
					case 'K': multiplier = 1024; break;
					case 'M': multiplier = 1024 * 1024; break;
					case 'G': multiplier = 1024 * 1024 * 1024; break;
					case 'B' : break;
					default : { Log::log_error(RD + n_line + "Invalid value for " Y "client_max_body_size" NC); return (1); }
				} str.erase(str.size() - 1);
			}

			long number; if (Utils::stol(str, number) || (str = Utils::ltos(number * multiplier)) == "") { Log::log_error(RD + n_line + "Invalid value for '" Y "client_max_body_size" RD "'" NC); return (1); }
			if (number < 1) { Log::log_error(RD + n_line + "Value for " Y "client_max_body_size" RD " cannot be " Y "lower" RD " than " Y "1 byte" NC); return (1); }
			if (number > 1024 * 1024 * 1024) { Log::log_error(RD + n_line + "Value for " Y "client_max_body_size" RD " cannot be " Y "greater" RD " than " Y "1GB" NC); return (1); }
			return (0);
		}

	#pragma endregion

	#pragma region Error Codes

		int Settings::parse_errors(const std::string & firstPart, const std::string & secondPart) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart);
			std::vector<std::string> errors; std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { Log::log_error(RD + n_line + "Empty value for " Y "error_page" NC); BadConfig = true; return (1); }
			std::string filePath = errors.back(); errors.pop_back();
			if (filePath.empty() || filePath[0] != '/') { Log::log_error(RD + n_line + "Invalid path for " Y "error_page" NC); BadConfig = true; return (1); }
			if (errors.size() > 1 && errors.back()[0] == '=') {
				long code; if (Utils::stol(errors.back().substr(1), code) || (error_codes.find(code) == error_codes.end())) {
					Log::log_error(RD + n_line + "Invalid status code " Y + errors.back().substr(1) + RD " for " Y "error_page" NC); BadConfig = true; }
				filePath = errors.back() + filePath; errors.pop_back();
			}

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (Utils::stol(*it, code) || (error_codes.find(code) == error_codes.end())) {
					Log::log_error(RD + n_line + "Invalid status code " Y +  *it + RD " for " Y "error_page" NC); BadConfig = true; }
				else global.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

		int Settings::parse_errors(const std::string & firstPart, const std::string & secondPart, VServer & VServ) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart);
			std::vector<std::string> errors; std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { Log::log_error(RD + n_line + "Empty value for " Y "error_page" NC); BadConfig = true; return (1); }
			std::string filePath = errors.back(); errors.pop_back();
			if (filePath.empty() || filePath[0] != '/') { Log::log_error(RD + n_line + "Invalid path for " Y "error_page" NC); BadConfig = true; return (1); }
			if (errors.size() > 1 && errors.back()[0] == '=') {
				long code; if (Utils::stol(errors.back().substr(1), code) || (error_codes.find(code) == error_codes.end())) {
					Log::log_error(RD + n_line + "Invalid status code " Y + errors.back().substr(1) + RD " for " Y "error_page" NC); BadConfig = true; }
				filePath = errors.back() + filePath; errors.pop_back();
			}

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (Utils::stol(*it, code) || (error_codes.find(code) == error_codes.end())) {
					Log::log_error(RD + n_line + "Invalid status code " Y +  *it + RD " for " Y "error_page" NC); BadConfig = true; }
				else VServ.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

		int Settings::parse_errors(const std::string & firstPart, const std::string & secondPart, Location & Loc) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart);
			std::vector<std::string> errors; std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { Log::log_error(RD + n_line + "Empty value for " Y "error_page" NC); BadConfig = true; return (1); }
			std::string filePath = errors.back(); errors.pop_back();
			if (filePath.empty() || filePath[0] != '/') { Log::log_error(RD + n_line + "Invalid path for " Y "error_page" NC); BadConfig = true; return (1); }
			if (errors.size() > 1 && errors.back()[0] == '=') {
				long code; if (Utils::stol(errors.back().substr(1), code) || (error_codes.find(code) == error_codes.end())) {
					Log::log_error(RD + n_line + "Invalid status code " Y + errors.back().substr(1) + RD " for " Y "error_page" NC); BadConfig = true; }
				filePath = errors.back() + filePath; errors.pop_back();
			}

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (Utils::stol(*it, code) || (error_codes.find(code) == error_codes.end())) {
					Log::log_error(RD + n_line + "Invalid status code " Y +  *it + RD " for " Y "error_page" NC); BadConfig = true; }
				else Loc.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

	#pragma endregion

	#pragma region Autoindex

		int Settings::parse_autoindex(std::string & str) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";

			if (str.empty()) {					Log::log_error(RD + n_line + "Empty value for " Y "autoindex" NC); return (1); }
			if (str != "on" && str != "off") {	Log::log_error(RD + n_line + "Invalid value for " Y "autoindex" NC); return (1); }
				
			return (0);
		}

	#pragma endregion

	#pragma region Index

		int Settings::parse_index(std::string & str) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";

			if (str.empty()) {		Log::log_error(RD + n_line + "Empty value for " Y "index" NC); return (1); }
			if (str[0] == '/') {	Log::log_error(RD + n_line + "Invalid value for " Y "index" NC); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Listen

		int Settings::parse_listen(std::string & str, VServer & VServ) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::string temp; std::istringstream stream(str); stream >> temp;
			std::string port; std::string::size_type slashPos = temp.find(':');
			
			if (temp.empty()) { 																		Log::log_error(RD + n_line + "Empty value for " Y "listen" NC); return (1); }

			if (slashPos != std::string::npos) {
				port = temp.substr(slashPos + 1);
				temp = temp.substr(0, slashPos);

				if (temp.empty()) {																		Log::log_error(RD + n_line + "Invalid IP for " Y "listen" NC); return (1); }
				slashPos = temp.find('/');
				if (slashPos != std::string::npos) {
					std::string ip = temp.substr(0, slashPos);
					std::string mask = temp.substr(slashPos + 1);
					if (ip.empty()) {																	Log::log_error(RD + n_line + "Invalid IP for " Y "listen" NC); return (1); }
					if (Utils::isValidIP(temp.substr(0, slashPos)) == false) {							Log::log_error(RD + n_line + "Invalid IP " Y + temp.substr(0, slashPos) + RD " for " Y "listen" NC); return (1); }
					if (mask.empty()) {																	Log::log_error(RD + n_line + "Invalid mask for " Y "listen" NC); return (1); }
					if (mask.find('.') != std::string::npos) { if (Utils::isValidIP(mask) == false) {	Log::log_error(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "listen" NC); return (1); }}
					else { long number; if (Utils::stol(mask, number) || number < 0 || number > 32) {	Log::log_error(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "listen" NC); return (1); }}
					VServ.add("IP", temp);
				} else {
					if (Utils::isValidIP(temp) == false) {												Log::log_error(RD + n_line + "Invalid IP " Y + temp + RD " for " Y "listen" NC); return (1); }
					VServ.add("IP", temp);
				}
			} else port = temp;

			if (port.empty()) { 																		Log::log_error(RD + n_line + "There is no port for " Y "listen" NC); return (1); }
			long number; if (Utils::stol(port, number)) { 												Log::log_error(RD + n_line + "Invalid port " Y + port + RD " for " Y "listen" NC); return (1); }
			if (number < 1) { 																			Log::log_error(RD + n_line + "Invalid port " Y + port + RD " for " Y "listen" RD " cannot be " Y "lower" RD " than " Y "1" NC); return (1); }
			if (number > 65535) { 																		Log::log_error(RD + n_line + "Invalid port " Y + port + RD " for " Y "listen" RD " cannot be " Y "greater" RD " than " Y "65535" NC); return (1); }
			VServ.add("Listen", port);
			return (0);
		}

	#pragma endregion

	#pragma region Return

		int Settings::parse_return(std::string & str) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(str); std::string code, path;

			stream >> code; stream >> path;
			if (code.empty()) { Log::log_error(RD + n_line + "Empty value for " Y "return" NC); return (1); }
			if (!(code[0] != '/' && !(code.size() >= 7 && code.compare(0, 7, "http://") == 0) && !(code.size() >= 8 && code.compare(0, 8, "https://") == 0))) {
				Log::log_error(RD + n_line + "Missing status code for " Y "return" NC); return (1); }

			long ncode; if (Utils::stol(code, ncode) || (error_codes.find(ncode) == error_codes.end())) {
				Log::log_error(RD + n_line + "Invalid status code " Y + code + RD " for " Y "return" NC); return (1); }

			if (!path.empty() && (path[0] != '/' && !(path.size() >= 7 && path.compare(0, 7, "http://") == 0) && !(path.size() >= 8 && path.compare(0, 8, "https://") == 0))) {
				Log::log_error(RD + n_line + "Invalid path for " Y "return" NC); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Alias

		int Settings::parse_alias(std::string & firstPart, std::string & str) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			
			if (str.empty()) {			Log::log_error(RD + n_line + "Empty value for " Y + "alias" + NC); return (1); }

			if (Utils::isFile(str)) return (parse_path(firstPart, str, true, true));
			else if (Utils::isDirectory(str)) return (parse_path(firstPart, str));
			else if (str[0] == '/') {	Log::log_error(RD + n_line + "The " Y "alias" RD " path " Y + str + RD " does not exist" NC); return (1); }
			else {						Log::log_error(RD + n_line + "Invalid value for " Y "alias" NC); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Try_Files

		int Settings::parse_try_files(std::string & str) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(str); std::string code;
			if (str.empty()) {			Log::log_error(RD + n_line + "Empty value for " Y + "try_files" + NC); return (1); }

			while (stream >> code) {
				if (!code.empty() && code[0] == '=') {
					long ncode; if (Utils::stol(code.substr(1), ncode) || (error_codes.find(ncode) == error_codes.end())) {
					Log::log_error(RD + n_line + "Invalid status code " Y + code.substr(1) + RD " for " Y "try_files" NC); return (1); }
				}
			}

			return (0);
		}

	#pragma endregion

	#pragma region CGI

		int Settings::parse_cgi(const std::string & firstPart, const std::string & secondPart) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart); std::vector<std::string> values; std::string value;

			while (stream >> value) values.push_back(value);
			if (values.size() < 2) { Log::log_error(RD + n_line + "Empty value for " Y "cgi" NC); BadConfig = true; return (1); }
			std::string filePath = values.back(); values.pop_back();
			if (filePath.empty() || filePath[0] != '/') { Log::log_error(RD + n_line + "Invalid path for " Y "cgi" NC); BadConfig = true; }
			if (parse_path(firstPart, filePath, true, true)) { BadConfig = true; }

			for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); ++it) {
				value = *it; if (value.empty() || value[0] != '.') { Log::log_error(RD + n_line + "Invalid extension " Y + value + RD " for " Y "cgi" NC); BadConfig = true; }
				else global.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

		int Settings::parse_cgi(const std::string & firstPart, const std::string & secondPart, VServer & VServ) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart); std::vector<std::string> values; std::string value;

			while (stream >> value) values.push_back(value);
			if (values.size() < 2) { Log::log_error(RD + n_line + "Empty value for " Y "cgi" NC); BadConfig = true; return (1); }
			std::string filePath = values.back(); values.pop_back();
			if (filePath.empty() || filePath[0] != '/') { Log::log_error(RD + n_line + "Invalid path for " Y "cgi" NC); BadConfig = true; }
			if (parse_path(filePath, value, true, true)) { BadConfig = true; }

			for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); ++it) {
				value = *it; if (value.empty() || value[0] != '.') { Log::log_error(RD + n_line + "Invalid extension " Y + value + RD " for " Y "cgi" NC); BadConfig = true; }
				else VServ.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

		int Settings::parse_cgi(const std::string & firstPart, const std::string & secondPart, Location & Loc) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart); std::vector<std::string> values; std::string value;

			while (stream >> value) values.push_back(value);
			if (values.size() < 2) { Log::log_error(RD + n_line + "Empty value for " Y "cgi" NC); BadConfig = true; return (1); }
			std::string filePath = values.back(); values.pop_back();
			if (filePath.empty() || filePath[0] != '/') { Log::log_error(RD + n_line + "Invalid path for " Y "cgi" NC); BadConfig = true; }
			if (parse_path(firstPart, filePath, true, true)) { BadConfig = true; }

			for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); ++it) {
				value = *it; if (value.empty() || value[0] != '.') { Log::log_error(RD + n_line + "Invalid extension " Y + value + RD " for " Y "cgi" NC); BadConfig = true; }
				else Loc.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}
	#pragma endregion

	#pragma region Allow

		int Settings::parse_allow(std::string & str) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::string temp; std::istringstream stream(str); stream >> temp;
			std::string::size_type slashPos;
			
			if (temp.empty()) { 																	Log::log_error(RD + n_line + "Empty value for " Y "allow" NC); return (1); }
			if (temp == "all") return (0);
			slashPos = temp.find('/');
			if (slashPos != std::string::npos) {
				std::string ip = temp.substr(0, slashPos);
				std::string mask = temp.substr(slashPos + 1);
				if (ip.empty()) {																	Log::log_error(RD + n_line + "Invalid IP for " Y "allow" NC); return (1); }
				if (Utils::isValidIP(temp.substr(0, slashPos)) == false) {							Log::log_error(RD + n_line + "Invalid IP " Y + temp.substr(0, slashPos) + RD " for " Y "allow" NC); return (1); }
				if (mask.empty()) {																	Log::log_error(RD + n_line + "Invalid mask for " Y "allow" NC); return (1); }
				if (mask.find('.') != std::string::npos) { if (Utils::isValidIP(mask) == false) {	Log::log_error(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "allow" NC); return (1); }}
				else { long number; if (Utils::stol(mask, number) || number < 0 || number > 32) {	Log::log_error(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "allow" NC); return (1); }}
			} else {
				if (Utils::isValidIP(temp) == false) {												Log::log_error(RD + n_line + "Invalid IP " Y + temp + RD " for " Y "allow" NC); return (1); }
			}
			return (0);
		}

	#pragma endregion

	#pragma region Deny

		int Settings::parse_deny(std::string & str) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			std::string temp; std::istringstream stream(str); stream >> temp;
			std::string::size_type slashPos;
			
			if (temp.empty()) { 																	Log::log_error(RD + n_line + "Empty value for " Y "deny" NC); return (1); }
			if (temp == "all") return (0);
			slashPos = temp.find('/');
			if (slashPos != std::string::npos) {
				std::string ip = temp.substr(0, slashPos);
				std::string mask = temp.substr(slashPos + 1);
				if (ip.empty()) {																	Log::log_error(RD + n_line + "Invalid IP for " Y "deny" NC); return (1); }
				if (Utils::isValidIP(temp.substr(0, slashPos)) == false) {							Log::log_error(RD + n_line + "Invalid IP " Y + temp.substr(0, slashPos) + RD " for " Y "deny" NC); return (1); }
				if (mask.empty()) {																	Log::log_error(RD + n_line + "Invalid mask for " Y "deny" NC); return (1); }
				if (mask.find('.') != std::string::npos) { if (Utils::isValidIP(mask) == false) {	Log::log_error(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "deny" NC); return (1); }}
				else { long number; if (Utils::stol(mask, number) || number < 0 || number > 32) {	Log::log_error(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "deny" NC); return (1); }}
			} else {
				if (Utils::isValidIP(temp) == false) {												Log::log_error(RD + n_line + "Invalid IP " Y + temp + RD " for " Y "deny" NC); return (1); }
			}
			return (0);
		}

	#pragma endregion

	#pragma region Limit_Except

		int Settings::parse_limit_except(std::string & str) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			
			if (str.empty()) {			Log::log_error(RD + n_line + "Empty value for " Y + "limit_except" + NC); return (1); }

			std::istringstream stream(str); std::string method;

			while (stream >> method) {
				if (method.empty()) {	Log::log_error(RD + n_line + "Empty method for " Y + "limit_except" + NC); return (1); }
				if (method != "GET" && method != "POST" && method != "DELETE" && method != "PUT" && method != "HEAD") {
					Log::log_error(RD + n_line + "Invalid method " Y + method + RD " for " Y + "limit_except" + NC); return (1); }
			}

			return (0);
		}

	#pragma endregion

	#pragma region Location

		int Settings::parse_location(std::string & str) {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			
			if (str.empty()) {										Log::log_error(RD + n_line + "Empty value for " Y "Location" NC); return (1); }

			std::istringstream stream(str); std::string exact, path;

			stream >> exact; stream >> path;

			if (exact == "=" && path.empty()) {						Log::log_error(RD + n_line + "Empty value for " Y "Location" NC); return (1); }
			if (exact != "=" && !path.empty()) {					Log::log_error(RD + n_line + "Invalid value " Y + exact + RD " for " Y + "Location" NC); return (1); }
			if (exact.empty()) {									Log::log_error(RD + n_line + "Empty value for " Y "Location" NC); return (1); }
			if (exact == "=" && !path.empty() && path[0] != '/') {	Log::log_error(RD + n_line + "Invalid value " Y + exact + RD "for " Y + "Location" NC); return (1); }
			if (exact != "=" && exact[0] != '/') {					Log::log_error(RD + n_line + "Invalid value " Y + exact + RD "for " Y + "Location" NC); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Invalid

		static int invalid_directive(std::string firstPart, int line_count, std::string Section = "Global") {
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
			if (firstPart == "access_log") return (0);
			if (firstPart == "error_log") return (0);
			if (firstPart == "root") return (0);
			if (firstPart == "uploads") return (0);
			if (firstPart == "client_max_body_size") return (0);
			if (firstPart == "autoindex") return (0);
			if (firstPart == "allow") return (0);
			if (firstPart == "deny") return (0);
			if (firstPart == "error_page") return (0);
			if (firstPart == "cgi") return (0);
			if (Section == "Global") {
				if (firstPart == "http") return (0);
			} else if (Section == "VServer") {
				if (firstPart == "server") return (0);
				if (firstPart == "listen") return (0);
				if (firstPart == "server_name") return (0);
				if (firstPart == "index") return (0);
				if (firstPart == "return") return (0);
			} else if (Section == "Location") {
				if (firstPart == "location") return (0);
				if (firstPart == "index") return (0);
				if (firstPart == "try_files") return (0);
				if (firstPart == "alias") return (0);
				if (firstPart == "internal") return (0);
				if (firstPart == "return") return (0);
			} else if (Section == "Method") {
				if (firstPart == "limit_except") return (0);
			}
			Log::log_error(RD + n_line + "Invalid directive " Y + firstPart + NC);
			return (1);
		}

	#pragma endregion

#pragma endregion

#pragma region Config File

	#pragma region Method

		int Settings::parser_method(std::ifstream & infile, std::string & line, VServer VServ, Location & Loc) {
			Method Met; int current_bracket = bracket_lvl; std::string tmp_line; std::string orig_line = line;
			do {
				line_count++; tmp_line = line;
				Utils::trim(line); if (line.empty()) {
					VServ.config.push_back(tmp_line);
					global.config.push_back(tmp_line);
					continue;
				}

				remove_semicolon(line, line_count);
				if (check_multiline(line, line_count))			BadConfig = true;
				if (remove_semicolon(line, line_count, true))	BadConfig = true;
				std::string firstPart, secondPart; std::istringstream stream(line);
				stream >> firstPart; Utils::trim(firstPart); Utils::toLower(firstPart);

				std::getline(stream, secondPart); Utils::trim(secondPart);

				if (tmp_line != orig_line) { VServ.config.push_back(tmp_line); global.config.push_back(tmp_line); }

				if (brackets(firstPart) + brackets(secondPart) < 0) {
					if (bracket_lvl < 0) return (0);
					if (bracket_lvl <= current_bracket) { Loc.add(Met); break; }
				}

				Utils::trim(firstPart); Utils::trim(secondPart);

				if (firstPart.empty()) continue;

				if (firstPart == "limit_except") {
					bool isSet = false;
					for (std::vector <Method>::iterator it = Loc.method.begin(); it != Loc.method.end(); ++it) {
						Method Met = *it;
						if (Met.get("limit_except") == secondPart) {
							std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
							Log::log_error(RD + n_line + "Directive " Y + firstPart + RD " is already set" + NC);
							BadConfig = true; isSet = true; break;
						}
					} if (isSet) continue;
				}

				if (Met.get(firstPart) != "" && firstPart != "allow" && firstPart != "deny") {
					std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
					Log::log_error(RD + n_line + "Directive " Y + firstPart + RD " is already set" + NC);
					BadConfig = true; continue;
				}

				if (firstPart == "limit_except" && parse_limit_except(secondPart))	BadConfig = true;
			
				if (firstPart == "allow" && parse_allow(secondPart))				BadConfig = true;
				if (firstPart == "deny" && parse_deny(secondPart))					BadConfig = true;
				if (firstPart == "return" && parse_return(secondPart))				BadConfig = true;
				if (invalid_directive(firstPart, line_count, "Method"))				BadConfig = true;
				else {
					if (firstPart == "allow" || firstPart == "deny")				Met.add(firstPart, secondPart, true);
					else 															Met.add(firstPart, secondPart);
				}
			} while (getline(infile, line));
			return (0);
		}

	#pragma endregion

	#pragma region Location

		int Settings::parser_location(std::ifstream & infile, std::string & line, VServer & VServ) {
			Location Loc; int current_bracket = bracket_lvl; std::string tmp_line; std::string orig_line = line;
			do {
				tmp_line = line;
				Utils::trim(line); if (line.empty()) {
					VServ.config.push_back(tmp_line);
					global.config.push_back(tmp_line);
					line_count++; continue;
				}

				remove_semicolon(line, line_count);
				if (check_multiline(line, line_count))			BadConfig = true;
				if (remove_semicolon(line, line_count, true))	BadConfig = true;
				std::string firstPart, secondPart; std::istringstream stream(line);
				stream >> firstPart; Utils::trim(firstPart); Utils::toLower(firstPart);

				if (firstPart == "limit_except") {
					VServ.config.push_back(tmp_line); global.config.push_back(tmp_line);
					parser_method(infile, line, VServ, Loc);
					firstPart = "ignore_me";
				}

				if (firstPart != "ignore_me") {
					line_count++;
					std::getline(stream, secondPart); Utils::trim(secondPart);
					if (!firstPart.empty() && tmp_line != orig_line) { VServ.config.push_back(tmp_line); global.config.push_back(tmp_line); }
				}

				if (brackets(firstPart) + brackets(secondPart) < 0) {
					if (bracket_lvl < 0) return (0);
					if (bracket_lvl <= current_bracket) { VServ.add(Loc); break; }
				}

				Utils::trim(firstPart); Utils::trim(secondPart);

				if (firstPart.empty() || firstPart == "ignore_me") continue;

				if (firstPart == "location") {
					bool isSet = false;
					for (std::vector <Location>::iterator it = VServ.location.begin(); it != VServ.location.end(); ++it) {
						Location Loc = *it;
						if (Loc.get("location") == secondPart) {
							std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
							Log::log_error(RD + n_line + "Directive " Y + firstPart + " " + secondPart + RD " is already set" + NC);
							BadConfig = true; isSet = true; break;
						}
					} if (isSet) continue;
				}

				if (Loc.get(firstPart) != "" && firstPart != "allow" && firstPart != "deny") {
					std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
					Log::log_error(RD + n_line + "Directive " Y + firstPart + RD " is already set" + NC);
					BadConfig = true; continue;
				}

				if (firstPart == "location" && parse_location(secondPart))						BadConfig = true;
				if (firstPart == "access_log" || firstPart == "error_log") parse_path(firstPart, secondPart, true, true);
				if (firstPart == "root" && parse_path(firstPart, secondPart, false, false))		BadConfig = true;
				if (firstPart == "uploads" && parse_path(firstPart, secondPart, false, false))	BadConfig = true;
				if (firstPart == "client_max_body_size" && parse_body_size(secondPart))			BadConfig = true;
				if (firstPart == "autoindex" && parse_autoindex(secondPart))					BadConfig = true;
				if (firstPart == "index" && parse_index(secondPart))							BadConfig = true;
				if (firstPart == "return" && parse_return(secondPart))							BadConfig = true;
				if (firstPart == "alias" && parse_alias(firstPart, secondPart))					BadConfig = true;
				if (firstPart == "try_files" && parse_try_files(secondPart))					BadConfig = true;
				if (firstPart == "allow" && parse_allow(secondPart))							BadConfig = true;
				if (firstPart == "deny" && parse_deny(secondPart))								BadConfig = true;
				if (firstPart == "error_page") parse_errors(firstPart, secondPart, Loc);
				if (firstPart == "cgi") parse_cgi(firstPart, secondPart, Loc);
				if (invalid_directive(firstPart, line_count, "Location"))						BadConfig = true;
				else {
					if (firstPart == "allow" || firstPart == "deny")			Loc.add(firstPart, secondPart, true);
					else if (firstPart != "error_page" && firstPart != "cgi")	Loc.add(firstPart, secondPart);
				}
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
					global.config.push_back(tmp_line);
					line_count++; continue;
				}

				remove_semicolon(line, line_count);
				if (check_multiline(line, line_count))			BadConfig = true;
				if (remove_semicolon(line, line_count, true))	BadConfig = true;
				std::string firstPart, secondPart; std::istringstream stream(line);				
				stream >> firstPart; Utils::trim(firstPart); Utils::toLower(firstPart);

				if (firstPart == "location") {
					VServ.config.push_back(tmp_line); global.config.push_back(tmp_line);
					parser_location(infile, line, VServ);
					firstPart = "ignore_me";
				}
					
				if (firstPart != "ignore_me") {
					line_count++;
					std::getline(stream, secondPart); Utils::trim(secondPart);
					if (!firstPart.empty() && tmp_line != orig_line) { VServ.config.push_back(tmp_line); global.config.push_back(tmp_line); }
				}

				if (brackets(firstPart) + brackets(secondPart) < 0) {
					if (bracket_lvl < 0) return (0);
					if (bracket_lvl <= current_bracket) { add(VServ); break; }
				}

				Utils::trim(firstPart); Utils::trim(secondPart);

				if (firstPart.empty() || firstPart == "ignore_me") continue;

				if (VServ.get(firstPart) != "" && firstPart != "allow" && firstPart != "deny") {
					std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
					Log::log_error(RD + n_line + "Directive " Y + firstPart + RD " is already set" + NC);
					continue;
				}

				if (firstPart == "access_log" || firstPart == "error_log") parse_path(firstPart, secondPart, true, true);
				if (firstPart == "root" && parse_path(firstPart, secondPart, false, false))		BadConfig = true;
				if (firstPart == "uploads" && parse_path(firstPart, secondPart, false, false))	BadConfig = true;
				if (firstPart == "client_max_body_size" && parse_body_size(secondPart))			BadConfig = true;
				if (firstPart == "autoindex" && parse_autoindex(secondPart))					BadConfig = true;
				if (firstPart == "index" && parse_index(secondPart))							BadConfig = true;
				if (firstPart == "return" && parse_return(secondPart))							BadConfig = true;
				if (firstPart == "listen" && parse_listen(secondPart, VServ))					BadConfig = true;
				if (firstPart == "allow" && parse_allow(secondPart))							BadConfig = true;
				if (firstPart == "deny" && parse_deny(secondPart))								BadConfig = true;
				if (firstPart == "limit_except" && parse_limit_except(secondPart))				BadConfig = true;
				if (firstPart == "error_page") parse_errors(firstPart, secondPart, VServ);
				if (firstPart == "cgi") parse_cgi(firstPart, secondPart, VServ);
				if (invalid_directive(firstPart, line_count, "VServer"))						BadConfig = true;
				else {
					if (firstPart == "allow" || firstPart == "deny") VServ.add(firstPart, secondPart, true);
					else if (firstPart != "server" && firstPart == "error_page" && firstPart != "listen" && firstPart != "cgi")
						VServ.add(firstPart, secondPart);
				}
			} while (getline(infile, line));
			return (0);
		}

	#pragma endregion

	#pragma region Global

		int Settings::parse_global(std::ifstream & infile, std::string & line) {
			global.config.push_back(line);
			Utils::trim(line); if (line.empty()) { line_count++; return (0); }

			remove_semicolon(line, line_count);
			if (check_multiline(line, line_count))			BadConfig = true;
			if (remove_semicolon(line, line_count, true))	BadConfig = true;
			std::string firstPart, secondPart; std::istringstream stream(line);
			stream >> firstPart; Utils::trim(firstPart); Utils::toLower(firstPart);
			
			if (firstPart == "http" && bracket_lvl != 0) {
				std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
				Log::log_error(RD + n_line + "Invalid directive " Y + firstPart + NC);
			}

			if (firstPart == "server") {
				parser_vserver(infile, line);
				firstPart = "";
			}

			if (!firstPart.empty()) {
				line_count++;
				std::getline(stream, secondPart); Utils::trim(secondPart);
			}

			brackets(firstPart); brackets(secondPart);

			Utils::trim(firstPart); Utils::trim(secondPart);

			if (firstPart.empty()) return (0);

			if (global.get(firstPart) != "" && firstPart != "allow" && firstPart != "deny") {
				std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] ";
				Log::log_error(RD + n_line + "Directive " Y + firstPart + RD " is already set" + NC);
				return (0);
			}

			if (firstPart == "access_log" || firstPart == "error_log") parse_path(firstPart, secondPart, true, true);
			if (firstPart == "root" && parse_path(firstPart, secondPart, false, false))		BadConfig = true;
			if (firstPart == "uploads" && parse_path(firstPart, secondPart, false, false))	BadConfig = true;
			if (firstPart == "client_max_body_size" && parse_body_size(secondPart))			BadConfig = true;
			if (firstPart == "autoindex" && parse_autoindex(secondPart))					BadConfig = true;
			if (firstPart == "index" && parse_index(secondPart))							BadConfig = true;
			if (firstPart == "allow" && parse_allow(secondPart))							BadConfig = true;
			if (firstPart == "deny" && parse_deny(secondPart))								BadConfig = true;
			if (firstPart == "error_page") parse_errors(firstPart, secondPart);
			if (firstPart == "cgi") parse_cgi(firstPart, secondPart);
			if (invalid_directive(firstPart, line_count))									BadConfig = true;
			else {
				if (firstPart == "allow" || firstPart == "deny")			global.add(firstPart, secondPart, true);
				else if (firstPart != "http" && firstPart != "error_page" && firstPart != "cgi")	global.add(firstPart, secondPart);
			}

			return (0);
		}

	#pragma endregion

#pragma endregion

//	si hay ; y { o } o ; log_error multi-line
//	brackets() y despues si no es http, server, location or limit_except y no termina en ; log_error
//	server_name check duplicates
//	server ip y puerto check duplicates
//	crear clase de variables con map $request_uri $uri
