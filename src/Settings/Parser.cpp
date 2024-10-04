/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 21:30:57 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/04 16:05:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Colors.hpp"
#include "Settings.hpp"

#include <unistd.h>																						//	For access() to checks the accessibility of a file or directory
#include <sys/stat.h>																					//	For stat() to retrieves information about a file or directory

#pragma region Logging

	void Settings::log_servers(std::string msg, VServer * VServ = NULL) {
		Utils::trim(msg); if (msg.empty()) return;
		msg = BLUE600 "[" LIME600 + timer.current_date() + " " EMERALD400 + timer.current_time() + BLUE600 "]  " NC + msg;
		if (VServ && !check_only) { VServ->log.error_add(msg); VServ->log.both_add(msg); }
		global.log.error_add(msg); global.log.both_add(msg);
	}

	void Settings::log_access_add(std::string msg) {
		Utils::trim(msg); if (msg.empty()) return;
		if (msg != "---")
			msg = BLUE600 "[" LIME600 + timer.current_date() + " " EMERALD400 + timer.current_time() + BLUE600 "]  " NC + msg;
		global.log.access_add(msg); global.log.both_add(msg);
	}

	void Settings::log_error_add(std::string msg) {
		Utils::trim(msg); if (msg.empty()) return;
		if (msg != "---")
			msg = BLUE600 "[" LIME600 + timer.current_date() + " " EMERALD400 + timer.current_time() + BLUE600 "]  " NC + msg;
		global.log.error_add(msg); global.log.both_add(msg);
	}

#pragma endregion

#pragma region Directives

	#pragma region Path

		int Settings::parse_path(const std::string & firstPart, std::string & str, bool isFile = false, bool check_path = false, bool check_write = false, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space; struct stat info;

			if (str.empty()) {										log_servers(RD + n_line + "Empty value for " Y + firstPart + NC, VServ); return (1); }
			if (str[0] == '~') str = Utils::expand_tilde(str);
			if (str[0] != '/') str = program_path + str;

			str = Utils::line_spaces_on(str);
			if (isFile && check_path) {
				std::string dir_path = str.substr(0, str.find_last_of('/'));
				if (stat(dir_path.c_str(), &info) != 0) {
					if (errno == ENOENT)							log_servers(RD + n_line + "The " Y + firstPart + RD " path " Y + dir_path + RD " does not exist" NC, VServ);
					else if (errno == EACCES)						log_servers(RD + n_line + "No permission to access " Y + dir_path + NC, VServ);
					else											log_servers(RD + n_line + "Cannot access " Y + dir_path + NC, VServ);
					return (1);
				} else {
					if (!(info.st_mode & S_IFDIR)) {				log_servers(RD + n_line + dir_path + RD " is not a valid directory" NC, VServ); return (1); }
					else if (firstPart != "access_log" && firstPart != "error_log" && access(str.c_str(), F_OK) != 0) {
																	log_servers(RD + n_line + "The " Y + firstPart + RD " path " Y + str + RD " does not exist" NC, VServ); return (1); }
					else if (check_write &&
						access(dir_path.c_str(), W_OK) != 0) {		log_servers(RD + n_line + "No write permission for " Y + dir_path + NC, VServ); return (1); }
					else if (check_write && access(str.c_str(), F_OK) == 0 &&
						access(str.c_str(), W_OK) != 0) {			log_servers(RD + n_line + "No write permission for " Y + str + NC, VServ); return (1); }
				}
			} else if (stat(str.c_str(), &info) != 0) {
				if (errno == ENOENT)								log_servers(RD + n_line + "The " Y + firstPart + RD " path " Y + str + RD " does not exist" NC, VServ);
				else if (errno == EACCES)							log_servers(RD + n_line + "No permission to access " Y + str + NC, VServ);
				else 												log_servers(RD + n_line + "Cannot access " Y + str + NC, VServ);
				return (1);
			} else {
				if (isFile && !(info.st_mode & S_IFREG)) { 			log_servers(RD + n_line + Y + str + RD " is not a valid file" NC, VServ); return (1); return (1); }
				else if (!isFile && !(info.st_mode & S_IFDIR)) { 	log_servers(RD + n_line + Y + str + RD " is not a valid directory" NC, VServ); return (1); }
				else if (access(str.c_str(), R_OK) != 0) {			log_servers(RD + n_line + "No permission to access " Y + str + NC, VServ); return (1); }
			}
			return (0);
		}

	#pragma endregion

	#pragma region Keep-Alive Timeout

		int Settings::parse_keepalive_timeout(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			if (str.empty()) { log_servers(RD + n_line + "Empty value for " Y "keepalive_timeout" NC, VServ); return (1); }

			long number; if (Utils::stol(str, number) || (str = Utils::ltos(number)) == "") { str = ""; log_servers(RD + n_line + "Invalid value for '" Y "keepalive_timeout" RD "'" NC, VServ); return (1); }
			if (number < 0) { str = ""; log_servers(RD + n_line + "Value for " Y "keepalive_timeout" RD " cannot be " Y "lower" RD " than " Y "0" NC, VServ); return (1); }
			if (number > 120) { str = ""; log_servers(RD + n_line + "Value for " Y "keepalive_timeout" RD " cannot be " Y "greater" RD " than " Y "120" NC, VServ); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Keep-Alive Requests

		int Settings::parse_keepalive_requests(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			if (str.empty()) { log_servers(RD + n_line + "Empty value for " Y "keepalive_requests" NC, VServ); return (1); }

			long number; if (Utils::stol(str, number) || (str = Utils::ltos(number)) == "") { str = ""; log_servers(RD + n_line + "Invalid value for '" Y "keepalive_requests" RD "'" NC, VServ); return (1); }
			if (number < 0) { str = ""; log_servers(RD + n_line + "Value for " Y "keepalive_requests" RD " cannot be " Y "lower" RD " than " Y "0" NC, VServ); return (1); }
			if (number > 5000) { str = ""; log_servers(RD + n_line + "Value for " Y "keepalive_requests" RD " cannot be " Y "greater" RD " than " Y "5000" NC, VServ); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Log RotateSize

		int Settings::parse_log_rotatesize(std::string & str, VServer * VServ = NULL) {
			long multiplier = 1; std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			if (str.empty()) { log_servers(RD + n_line + "Empty value for " Y "log_rotatesize" NC, VServ); return (1); }
			if (str.size() > 1 && !std::isdigit(str[str.size() - 1]) && !std::isdigit(str[str.size() - 2])) {
				if ((std::tolower(str[str.size() - 2]) == 'k' || std::tolower(str[str.size() - 2]) == 'm') && std::tolower(str[str.size() - 1]) == 'b') str.erase(str.size() - 1);
			}
			if (!std::isdigit(str[str.size() - 1])) {
				switch (std::tolower(str[str.size() - 1])) {
					case 'k': multiplier = 1024; break;
					case 'm': multiplier = 1024 * 1024; break;
					case 'b' : break;
					default : { str = ""; log_servers(RD + n_line + "Invalid value for " Y "log_rotatesize" NC, VServ); return (1); }
				} str.erase(str.size() - 1);
			}

			long number; if (Utils::stol(str, number) || (str = Utils::ltos(number * multiplier)) == "") { str = ""; log_servers(RD + n_line + "Invalid value for '" Y "log_rotatesize" RD "'" NC, VServ); return (1); }
			if (number * multiplier != 0 && number * multiplier < 102400) { str = ""; log_servers(RD + n_line + "Value for " Y "log_rotatesize" RD " cannot be " Y "lower" RD " than " Y "1 KB" NC, VServ); return (1); }
			if (number * multiplier > 100 * 1024 * 1024) { str = ""; log_servers(RD + n_line + "Value for " Y "log_rotatesize" RD " cannot be " Y "greater" RD " than " Y "100 MB" NC, VServ); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Log Rotate

		int Settings::parse_log_rotate(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			if (str.empty()) { log_servers(RD + n_line + "Empty value for " Y "log_rotate" NC, VServ); return (1); }

			long number; if (Utils::stol(str, number) || (str = Utils::ltos(number)) == "") { str = ""; log_servers(RD + n_line + "Invalid value for '" Y "log_rotate" RD "'" NC, VServ); return (1); }
			if (number < 0) 	{ str = ""; log_servers(RD + n_line + "Value for " Y "log_rotate" RD " cannot be " Y "lower" RD " than " Y "0" NC, VServ); return (1); }
			if (number > 100)	{ str = ""; log_servers(RD + n_line + "Value for " Y "log_rotate" RD " cannot be " Y "greater" RD " than " Y "100" NC, VServ); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Body Size

		int Settings::parse_body_size(std::string & str, VServer * VServ = NULL) {
			long multiplier = 1; std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			if (str.empty()) { log_servers(RD + n_line + "Empty value for " Y "client_max_body_size" NC, VServ); return (1); }
			if (str.size() > 1 && !std::isdigit(str[str.size() - 1]) && !std::isdigit(str[str.size() - 2])) {
				if ((std::tolower(str[str.size() - 2]) == 'k' || std::tolower(str[str.size() - 2]) == 'm' || std::tolower(str[str.size() - 2]) == 'g') && std::tolower(str[str.size() - 1]) == 'b') str.erase(str.size() - 1);
			}
			if (!std::isdigit(str[str.size() - 1])) {
				switch (std::tolower(str[str.size() - 1])) {
					case 'k': multiplier = 1024; break;
					case 'm': multiplier = 1024 * 1024; break;
					case 'g': multiplier = 1024 * 1024 * 1024; break;
					case 'b' : break;
					default : { str = ""; log_servers(RD + n_line + "Invalid value for " Y "client_max_body_size" NC, VServ); return (1); }
				} str.erase(str.size() - 1);
			}

			long number; if (Utils::stol(str, number) || (str = Utils::ltos(number * multiplier)) == "") { str = ""; log_servers(RD + n_line + "Invalid value for '" Y "client_max_body_size" RD "'" NC, VServ); return (1); }
			if (number * multiplier < 1) { str = ""; log_servers(RD + n_line + "Value for " Y "client_max_body_size" RD " cannot be " Y "lower" RD " than " Y "1 byte" NC, VServ); return (1); }
			if (number * multiplier > 1024 * 1024 * 1024) { str = ""; log_servers(RD + n_line + "Value for " Y "client_max_body_size" RD " cannot be " Y "greater" RD " than " Y "1 GB" NC, VServ); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Error Codes

		int Settings::parse_errors(const std::string & firstPart, const std::string & secondPart, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			std::istringstream stream(Utils::line_spaces_off(secondPart));
			std::vector<std::string> errors; std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { log_servers(RD + n_line + "Empty value for " Y "error_page" NC, VServ); global.bad_config = true; return (1); }
			std::string filePath = errors.back(); errors.pop_back();
			filePath = Utils::line_spaces_on(filePath);
			Utils::trim(filePath);
			if (filePath.empty() || filePath[0] != '/') { log_servers(RD + n_line + "Invalid path for " Y "error_page" NC, VServ); global.bad_config = true; return (1); }
			if (errors.size() > 1 && errors.back()[0] == '=') {
				long code; if (Utils::stol(errors.back().substr(1), code) || (error_codes.find(code) == error_codes.end())) {
					log_servers(RD + n_line + "Invalid status code " Y + errors.back().substr(1) + RD " for " Y "error_page" NC, VServ); global.bad_config = true; }
				filePath = errors.back() + filePath; errors.pop_back();
			}

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (Utils::stol(*it, code) || (error_codes.find(code) == error_codes.end())) {
					log_servers(RD + n_line + "Invalid status code " Y +  *it + RD " for " Y "error_page" NC, VServ); global.bad_config = true; }
				else global.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

		int Settings::parse_errors(const std::string & firstPart, const std::string & secondPart, VServer & VServ) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			std::istringstream stream(Utils::line_spaces_off(secondPart));
			std::vector<std::string> errors; std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { log_servers(RD + n_line + "Empty value for " Y "error_page" NC, &VServ); VServ.bad_config = true; return (1); }
			std::string filePath = errors.back(); errors.pop_back();
			filePath = Utils::line_spaces_on(filePath);
			Utils::trim(filePath);
			if (filePath.empty() || filePath[0] != '/') { log_servers(RD + n_line + "Invalid path for " Y "error_page" NC, &VServ); VServ.bad_config = true; return (1); }
			if (errors.size() > 1 && errors.back()[0] == '=') {
				long code; if (Utils::stol(errors.back().substr(1), code) || (error_codes.find(code) == error_codes.end())) {
					log_servers(RD + n_line + "Invalid status code " Y + errors.back().substr(1) + RD " for " Y "error_page" NC, &VServ); VServ.bad_config = true; }
				filePath = errors.back() + filePath; errors.pop_back();
			}

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (Utils::stol(*it, code) || (error_codes.find(code) == error_codes.end())) {
					log_servers(RD + n_line + "Invalid status code " Y +  *it + RD " for " Y "error_page" NC, &VServ); VServ.bad_config = true; }
				else VServ.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

		int Settings::parse_errors(const std::string & firstPart, const std::string & secondPart, Location & Loc) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			std::istringstream stream(Utils::line_spaces_off(secondPart));
			std::vector<std::string> errors; std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { log_servers(RD + n_line + "Empty value for " Y "error_page" NC, Loc.VServ); Loc.VServ->bad_config = true; return (1); }
			std::string filePath = errors.back(); errors.pop_back();
			filePath = Utils::line_spaces_on(filePath);
			Utils::trim(filePath);
			if (filePath.empty() || filePath[0] != '/') {
				log_servers(RD + n_line + "Invalid path for " Y "error_page" NC, Loc.VServ);
				Loc.VServ->bad_config = true; return (1);
			}
			if (errors.size() > 1 && errors.back()[0] == '=') {
				long code; if (Utils::stol(errors.back().substr(1), code) || (error_codes.find(code) == error_codes.end())) {
					log_servers(RD + n_line + "Invalid status code " Y + errors.back().substr(1) + RD " for " Y "error_page" NC, Loc.VServ); Loc.VServ->bad_config = true; }
				filePath = errors.back() + filePath; errors.pop_back();
			}

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (Utils::stol(*it, code) || (error_codes.find(code) == error_codes.end())) {
					log_servers(RD + n_line + "Invalid status code " Y +  *it + RD " for " Y "error_page" NC, Loc.VServ); Loc.VServ->bad_config = true; }
				else Loc.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

	#pragma endregion

	#pragma region Autoindex

		int Settings::parse_autoindex(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			if (str.empty()) {					log_servers(RD + n_line + "Empty value for " Y "autoindex" NC, VServ); return (1); }
			if (str != "on" && str != "off") {	log_servers(RD + n_line + "Invalid value for " Y "autoindex" NC, VServ); return (1); }
				
			return (0);
		}

	#pragma endregion

	#pragma region Index

		int Settings::parse_index(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			if (str.empty()) {		log_servers(RD + n_line + "Empty value for " Y "index" NC, VServ); return (1); }
			if (str[0] == '/') {	log_servers(RD + n_line + "Invalid value for " Y "index" NC, VServ); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Listen

		int Settings::parse_listen(std::string & str, VServer & VServ) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
			std::string temp; std::istringstream stream(str); stream >> temp;
			std::string IP; std::string port; std::string::size_type slashPos = temp.find(':');	
			
			if (temp.empty()) { 																		log_servers(RD + n_line + "Empty value for " Y "listen" NC, &VServ); return (1); }

			if (slashPos != std::string::npos) {
				port = temp.substr(slashPos + 1);
				temp = temp.substr(0, slashPos);

				if (temp.empty()) {																		log_servers(RD + n_line + "Invalid IP for " Y "listen" NC, &VServ); return (1); }
				slashPos = temp.find('/');
				if (slashPos != std::string::npos) {
					std::string ip = temp.substr(0, slashPos);
					std::string mask = temp.substr(slashPos + 1);
					if (ip.empty()) {																	log_servers(RD + n_line + "Invalid IP for " Y "listen" NC, &VServ); return (1); }
					if (Utils::isValidIP(temp.substr(0, slashPos)) == false) {							log_servers(RD + n_line + "Invalid IP " Y + temp.substr(0, slashPos) + RD " for " Y "listen" NC, &VServ); return (1); }
					if (mask.empty()) {																	log_servers(RD + n_line + "Invalid mask for " Y "listen" NC, &VServ); return (1); }
					if (mask.find('.') != std::string::npos) { if (Utils::isValidMask(mask) == false) {	log_servers(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "listen" NC, &VServ); return (1); }}
					else { long number; if (Utils::stol(mask, number) || number < 0 || number > 32) {	log_servers(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "listen" NC, &VServ); return (1); }}
					IP = temp;
				} else {
					if (Utils::isValidIP(temp) == false) {												log_servers(RD + n_line + "Invalid IP " Y + temp + RD " for " Y "listen" NC, &VServ); return (1); }
					IP = temp;
				}
			} else port = temp;

			if (port.empty()) { 																		log_servers(RD + n_line + "There is no port for " Y "listen" NC, &VServ); return (1); }
			long number; if (Utils::stol(port, number)) { 												log_servers(RD + n_line + "Invalid port " Y + port + RD " for " Y "listen" NC, &VServ); return (1); }
			if (number < 1) { 																			log_servers(RD + n_line + "Invalid port " Y + port + RD " for " Y "listen" RD " cannot be " Y "lower" RD " than " Y "1" NC, &VServ); return (1); }
			if (number > 65535) { 																		log_servers(RD + n_line + "Invalid port " Y + port + RD " for " Y "listen" RD " cannot be " Y "greater" RD " than " Y "65535" NC, &VServ); return (1); }
			if (IP.empty()) Utils::add_address("0.0.0.0", number, VServ);
			else Utils::add_address(IP, number, VServ);
		
			return (0);
		}

	#pragma endregion

	#pragma region Return

		int Settings::parse_return(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
			std::istringstream stream(str); std::string code, path;

			stream >> code;
			std::getline(stream, path);
			Utils::trim(path);
			path = Utils::line_spaces_on(path);

			if (code.empty()) { log_servers(RD + n_line + "Empty value for " Y "return" NC, VServ); return (1); }
			if (!(code[0] != '/' && !(code.size() >= 7 && code.compare(0, 7, "http://") == 0) && !(code.size() >= 8 && code.compare(0, 8, "https://") == 0))) {
				log_servers(RD + n_line + "Missing status code for " Y "return" NC, VServ); return (1); }

			long ncode; if (Utils::stol(code, ncode) || (error_codes.find(ncode) == error_codes.end())) {
				log_servers(RD + n_line + "Invalid status code " Y + code + RD " for " Y "return" NC, VServ); return (1); }

			if (!path.empty() && (path[0] != '/' && !(path.size() >= 7 && path.compare(0, 7, "http://") == 0) && !(path.size() >= 8 && path.compare(0, 8, "https://") == 0))) {
				log_servers(RD + n_line + "Invalid path for " Y "return" NC, VServ); return (1); }

			str = Utils::line_spaces_on(str);
			return (0);
		}

	#pragma endregion

	#pragma region Alias

		int Settings::parse_alias(std::string & firstPart, std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
			
			str = Utils::line_spaces_on(str);
			if (str.empty()) {			log_servers(RD + n_line + "Empty value for " Y + "alias" + NC, VServ); return (1); }

			if (Utils::isFile(str)) return (parse_path(firstPart, str, true, true));
			else if (Utils::isDirectory(str)) return (parse_path(firstPart, str));
			else if (str[0] == '/' || str[0] == '~') {	log_servers(RD + n_line + "The " Y "alias" RD " path " Y + str + RD " does not exist" NC, VServ); return (1); }
			else {						log_servers(RD + n_line + "Invalid value for " Y "alias" NC, VServ); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Try_Files

		int Settings::parse_try_files(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			str = Utils::line_spaces_off(str);
			std::istringstream stream(str); std::string code;
			if (str.empty()) {			log_servers(RD + n_line + "Empty value for " Y + "try_files" + NC, VServ); return (1); }

			while (stream >> code) {
				if (!code.empty() && code[0] == '=') {
					long ncode; if (Utils::stol(code.substr(1), ncode) || (error_codes.find(ncode) == error_codes.end())) {
					log_servers(RD + n_line + "Invalid status code " Y + code.substr(1) + RD " for " Y "try_files" NC, VServ); return (1); }
				}
			}

			return (0);
		}

	#pragma endregion

	#pragma region CGI

		int Settings::parse_cgi(const std::string & firstPart, const std::string & secondPart, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
			
			std::istringstream stream(Utils::line_spaces_off(secondPart)); std::vector<std::string> values; std::string value;

			while (stream >> value) values.push_back(value);
			if (values.size() == 0) { log_servers(RD + n_line + "Empty value for " Y "CGI" NC, VServ); global.bad_config = true; return (1); }
			std::string filePath = values.back(); values.pop_back();
			filePath = Utils::line_spaces_on(filePath);
			if (values.size() == 0) { log_servers(RD + n_line + "Missing value or path for " Y "CGI" NC, VServ); global.bad_config = true; return (1); }
			//if (filePath.empty() || (filePath[0] != '/' && filePath[0] != '~' && Utils::strToLower(filePath) != "self-cgi")) { log_servers(RD + n_line + "Invalid path for " Y "CGI" NC, VServ); global.bad_config = true; return (1); }
			if (Utils::strToLower(filePath) != "self-cgi" && parse_path("CGI", filePath, true, true)) { global.bad_config = true; }

			for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); ++it) { value = *it;
				std::string lvalue = value; Utils::toLower(lvalue);
				if (!value.empty() && value[0] != '.' && lvalue != "dir" && lvalue != "head" && lvalue != "get" && lvalue != "post" && lvalue != "put" && lvalue != "patch" && lvalue != "delete") {
					if (value[0] == '.')	log_servers(RD + n_line + "Invalid extension " Y + value + RD " for " Y "CGI" NC, VServ);
					else					log_servers(RD + n_line + "Invalid method " Y + Utils::strToUpper(value) + RD " for " Y "CGI" NC, VServ);
					global.bad_config = true;
				} else if (value[0] != '.' && Utils::strToLower(filePath) == "self-cgi") { log_servers(RD + n_line + "Cannot use " Y "self-CGI" RD " for the method " Y + Utils::strToUpper(value) + NC, VServ); global.bad_config = true;
				} else if (lvalue == "dir" || lvalue == "head" || lvalue == "get" || lvalue == "post" || lvalue == "put" || lvalue == "patch" || lvalue == "delete")
					global.add(Utils::strToLower(firstPart) + " " + Utils::strToLower(*it), filePath);
				else global.add(Utils::strToLower(firstPart) + " " + *it, filePath);
			}

			return (0);
		}

		int Settings::parse_cgi(const std::string & firstPart, const std::string & secondPart, VServer & VServ) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
			std::istringstream stream(Utils::line_spaces_off(secondPart)); std::vector<std::string> values; std::string value;

			while (stream >> value) values.push_back(value);
			if (values.size() == 0) { log_servers(RD + n_line + "Empty value for " Y "CGI" NC, &VServ); VServ.bad_config = true; return (1); }
			std::string filePath = values.back(); values.pop_back();
			filePath = Utils::line_spaces_on(filePath);
			if (values.size() == 0) { log_servers(RD + n_line + "Missing value or path for " Y "CGI" NC, &VServ); VServ.bad_config = true; return (1); }
			//if (filePath.empty() || (filePath[0] != '/' && filePath[0] != '~' && Utils::strToLower(filePath) != "self-cgi")) { log_servers(RD + n_line + "Invalid path for " Y "CGI" NC, &VServ); VServ.bad_config = true; return (1); }
			if (Utils::strToLower(filePath) != "self-cgi" && parse_path("CGI", filePath, true, true)) { VServ.bad_config = true; }

			for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); ++it) { value = *it;
				std::string lvalue = value; Utils::toLower(lvalue);
				if (!value.empty() && value[0] != '.' && lvalue != "dir" && lvalue != "head" && lvalue != "get" && lvalue != "post" && lvalue != "put" && lvalue != "patch" && lvalue != "delete") {
					if (value[0] == '.')	log_servers(RD + n_line + "Invalid extension " Y + value + RD " for " Y "CGI" NC, &VServ);
					else					log_servers(RD + n_line + "Invalid method " Y + Utils::strToUpper(value) + RD " for " Y "CGI" NC, &VServ);
					VServ.bad_config = true;
				} else if (value[0] != '.' && Utils::strToLower(filePath) == "self-cgi") { log_servers(RD + n_line + "Cannot use " Y "self-CGI" RD " for the method " Y + Utils::strToUpper(value) + NC, &VServ); VServ.bad_config = true;
				} else if (lvalue == "dir" || lvalue == "head" || lvalue == "get" || lvalue == "post" || lvalue == "put" || lvalue == "patch" || lvalue == "delete")
					VServ.add(Utils::strToLower(firstPart) + " " + Utils::strToLower(*it), filePath);
				else VServ.add(Utils::strToLower(firstPart) + " " + *it, filePath);
			}

			return (0);
		}

		int Settings::parse_cgi(const std::string & firstPart, const std::string & secondPart, Location & Loc) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
			std::istringstream stream(Utils::line_spaces_off(secondPart)); std::vector<std::string> values; std::string value;

			while (stream >> value) values.push_back(value);
			if (values.size() == 0) { log_servers(RD + n_line + "Empty value for " Y "CGI" NC, Loc.VServ); Loc.VServ->bad_config = true; return (1); }
			std::string filePath = values.back(); values.pop_back();
			filePath = Utils::line_spaces_on(filePath);
			if (values.size() == 0) { log_servers(RD + n_line + "Missing value or path for " Y "CGI" NC, Loc.VServ); Loc.VServ->bad_config = true; return (1); }
			//if (filePath.empty() || (filePath[0] != '/' && filePath[0] != '~' && Utils::strToLower(filePath) != "self-cgi")) { log_servers(RD + n_line + "Invalid path for " Y "CGI" NC, Loc.VServ); Loc.VServ->bad_config = true; return (1); }
			if (Utils::strToLower(filePath) != "self-cgi" && parse_path("CGI", filePath, true, true)) { Loc.VServ->bad_config = true; }

			for (std::vector<std::string>::iterator it = values.begin(); it != values.end(); ++it) { value = *it;
				std::string lvalue = value; Utils::toLower(lvalue);
				if (!value.empty() && value[0] != '.' && lvalue != "dir" && lvalue != "head" && lvalue != "get" && lvalue != "post" && lvalue != "put" && lvalue != "patch" && lvalue != "delete") {
					if (value[0] == '.')	log_servers(RD + n_line + "Invalid extension " Y + value + RD " for " Y "CGI" NC, Loc.VServ);
					else					log_servers(RD + n_line + "Invalid method " Y + Utils::strToUpper(value) + RD " for " Y "CGI" NC, Loc.VServ);
					Loc.VServ->bad_config = true;
				} else if (value[0] != '.' && Utils::strToLower(filePath) == "self-cgi") { log_servers(RD + n_line + "Cannot use " Y "self-CGI" RD " for the method " Y + Utils::strToUpper(value) + NC, Loc.VServ); Loc.VServ->bad_config = true;
				} else if (lvalue == "dir" || lvalue == "head" || lvalue == "get" || lvalue == "post" || lvalue == "put" || lvalue == "patch" || lvalue == "delete")
					Loc.add(Utils::strToLower(firstPart) + " " + Utils::strToLower(*it), filePath);
				else Loc.add(Utils::strToLower(firstPart) + " " + *it, filePath);
			}

			return (0);
		}
	#pragma endregion

	#pragma region Allow

		int Settings::parse_allow(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
			std::string temp; std::istringstream stream(str); stream >> temp;
			std::string::size_type slashPos;
			
			if (temp.empty()) { 																	log_servers(RD + n_line + "Empty value for " Y "allow" NC, VServ); return (1); }
			if (temp == "all") return (0);
			slashPos = temp.find('/');
			if (slashPos != std::string::npos) {
				std::string ip = temp.substr(0, slashPos);
				std::string mask = temp.substr(slashPos + 1);
				if (ip.empty()) {																	log_servers(RD + n_line + "Invalid IP for " Y "allow" NC, VServ); return (1); }
				if (Utils::isValidIP(temp.substr(0, slashPos)) == false) {							log_servers(RD + n_line + "Invalid IP " Y + temp.substr(0, slashPos) + RD " for " Y "allow" NC, VServ); return (1); }
				if (mask.empty()) {																	log_servers(RD + n_line + "Invalid mask for " Y "allow" NC, VServ); return (1); }
				if (mask.find('.') != std::string::npos) { if (Utils::isValidIP(mask) == false) {	log_servers(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "allow" NC, VServ); return (1); }}
				else { long number; if (Utils::stol(mask, number) || number < 0 || number > 32) {	log_servers(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "allow" NC, VServ); return (1); }}
			} else {
				if (Utils::isValidIP(temp) == false) {												log_servers(RD + n_line + "Invalid IP " Y + temp + RD " for " Y "allow" NC, VServ); return (1); }
			}
			return (0);
		}

	#pragma endregion

	#pragma region Deny

		int Settings::parse_deny(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
			std::string temp; std::istringstream stream(str); stream >> temp;
			std::string::size_type slashPos;
			
			if (temp.empty()) { 																	log_servers(RD + n_line + "Empty value for " Y "deny" NC, VServ); return (1); }
			if (temp == "all") return (0);
			slashPos = temp.find('/');
			if (slashPos != std::string::npos) {
				std::string ip = temp.substr(0, slashPos);
				std::string mask = temp.substr(slashPos + 1);
				if (ip.empty()) {																	log_servers(RD + n_line + "Invalid IP for " Y "deny" NC, VServ); return (1); }
				if (Utils::isValidIP(temp.substr(0, slashPos)) == false) {							log_servers(RD + n_line + "Invalid IP " Y + temp.substr(0, slashPos) + RD " for " Y "deny" NC, VServ); return (1); }
				if (mask.empty()) {																	log_servers(RD + n_line + "Invalid mask for " Y "deny" NC, VServ); return (1); }
				if (mask.find('.') != std::string::npos) { if (Utils::isValidIP(mask) == false) {	log_servers(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "deny" NC, VServ); return (1); }}
				else { long number; if (Utils::stol(mask, number) || number < 0 || number > 32) {	log_servers(RD + n_line + "Invalid mask " Y + temp.substr(slashPos + 1) + RD " for " Y "deny" NC, VServ); return (1); }}
			} else {
				if (Utils::isValidIP(temp) == false) {												log_servers(RD + n_line + "Invalid IP " Y + temp + RD " for " Y "deny" NC, VServ); return (1); }
			}
			return (0);
		}

	#pragma endregion

	#pragma region Method

		int Settings::parse_method(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
			
			if (str.empty()) {			log_servers(RD + n_line + "Empty value for " Y + "method" NC, VServ); return (1); }

			std::istringstream stream(str); std::string method;

			while (stream >> method) {
				std::string lmethod = method; Utils::toLower(lmethod);
				if (method.empty()) {	log_servers(RD + n_line + "Empty method for " Y + "method" NC, VServ); return (1); }
				if (lmethod != "head" && lmethod != "get" && lmethod != "post" && lmethod != "put" && lmethod != "patch" && lmethod != "delete") {
					log_servers(RD + n_line + "Invalid method " Y + method + RD " for " Y + "method" NC, VServ); return (1); }
			}

			return (0);
		}

	#pragma endregion

	#pragma region Location

		int Settings::parse_location(std::string & str, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
			
			if (str.empty()) {															log_servers(RD + n_line + "Empty value for " Y "Location" NC, VServ); return (1); }

			str = Utils::line_spaces_off(str);
			std::istringstream stream(str); std::string exact, path;

			stream >> exact; stream >> path;

			exact = Utils::line_spaces_on(exact);
			path = Utils::line_spaces_on(path);
			if ((exact == "=" || exact == "~*") && path.empty()) {						log_servers(RD + n_line + "Empty value for " Y "Location" NC, VServ); return (1); }
			if (exact != "=" && exact != "~*" && !path.empty()) {						log_servers(RD + n_line + "Invalid value " Y + exact + RD " for " Y + "Location" NC, VServ); return (1); }
			if (exact.empty()) {														log_servers(RD + n_line + "Empty value for " Y "Location" NC, VServ); return (1); }
			if ((exact == "=" || exact == "~*") && !path.empty() && path[0] != '/') {	log_servers(RD + n_line + "Invalid path " Y + path + RD " for " Y + "Location" NC, VServ); return (1); }
			if (exact != "=" && exact != "~*" && exact[0] != '/') {						log_servers(RD + n_line + "Invalid value " Y + exact + RD " for " Y + "Location" NC, VServ); return (1); }
			str = Utils::line_spaces_on(str);

			return (0);
		}

	#pragma endregion

	#pragma region Repeated

		int Settings::repeated_directive(const std::string & str, const std::vector<std::pair<std::string, std::string> > & data, int line_count, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			if (str.empty() || data.size() == 0 || str == "listen" || str == "allow" || str == "deny" || Utils::strToLower(str) == "cgi") return (0);

			for (std::vector <std::pair<std::string, std::string> >::const_iterator it = data.begin(); it != data.end(); ++it)
				if (it->first == str) { log_servers(RD + n_line + "Directive " Y + str + RD " repeated" NC, VServ); return (1); }

			return (0);
		}

	#pragma endregion

	#pragma region Invalid

		int Settings::invalid_directive(std::string firstPart, int line_count, int section, VServer * VServ = NULL) {
			std::string space = ""; if (line_count - 1 < 10) space = " ";
			std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;

			if (firstPart.empty()) return (0);
			if (section == GLOBAL || section == SERVER || section == LOCATION) {
				if (firstPart == "access_log") return (0);
				if (firstPart == "error_log") return (0);
				if (firstPart == "log_rotatesize") return (0);
				if (firstPart == "log_rotate") return (0);
				if (firstPart == "root") return (0);
				if (firstPart == "index") return (0);
				if (firstPart == "body_maxsize") return (0);
				if (firstPart == "autoindex") return (0);
				if (firstPart == "allow") return (0);
				if (firstPart == "deny") return (0);
				if (firstPart == "error_page") return (0);
				if (Utils::strToLower(firstPart) == "cgi") return (0);
				if (firstPart == "return") return (0);
			}
			if (section == GLOBAL) {
				if (firstPart == "http") return (0);
				if (firstPart == "keepalive_timeout") return (0);
				if (firstPart == "keepalive_requests") return (0);
			} else if (section == SERVER) {
				if (firstPart == "server") return (0);
				if (firstPart == "server_name") return (0);
				if (firstPart == "listen") return (0);
			} else if (section == LOCATION) {
				if (firstPart == "location") return (0);
				if (firstPart == "try_files") return (0);
				if (firstPart == "alias") return (0);
				// if (firstPart == "internal") return (0);
			} else if (section == METHOD) {
				if (firstPart == "method") return (0);
				if (firstPart == "allow") return (0);
				if (firstPart == "deny") return (0);
				if (firstPart == "return") return (0);
			}
			log_servers(RD + n_line + "Invalid directive " Y + firstPart + NC, VServ); return (1);
		}

	#pragma endregion

	#pragma region Missing

		void Settings::missing_directive(VServer & VServ) {
			if (VServ.get("listen") == "") {
				std::string msg = RD "Missing directive " Y "listen" NC;
				msg = BLUE600 "[" LIME600 + timer.current_date() + " " EMERALD400 + timer.current_time() + BLUE600 "]  " NC + msg;
				if (!check_only) { VServ.log.error_add(msg); VServ.log.both_add(msg); }

				msg = RD "Missing directive " Y "listen" RD " for a virtual server" NC;
				msg = BLUE600 "[" LIME600 + timer.current_date() + " " EMERALD400 + timer.current_time() + BLUE600 "]  " NC + msg;
				if (!check_only) { global.log.error_add(msg); global.log.both_add(msg); }

				VServ.bad_config = true;
			}

			bool Loc_root = true, VServ_root = (!VServ.get("root").empty() || !global.get("root").empty());
			for (std::deque<Location>::iterator it = VServ.location.begin(); it != VServ.location.end(); ++it)
				if (it->get("root").empty() && !VServ_root) { Loc_root = false; break; }

			if (!VServ_root && (VServ.location.size() == 0 || Loc_root == false)) {
				std::string msg = RD "Missing directive " Y "root" NC;
				msg = BLUE600 "[" LIME600 + timer.current_date() + " " EMERALD400 + timer.current_time() + BLUE600 "]  " NC + msg;
				if (!check_only) { VServ.log.error_add(msg); VServ.log.both_add(msg); }

				msg = RD "Missing directive " Y "root" RD " for a virtual server" NC;
				msg = BLUE600 "[" LIME600 + timer.current_date() + " " EMERALD400 + timer.current_time() + BLUE600 "]  " NC + msg;
				if (!check_only) { global.log.error_add(msg); global.log.both_add(msg); }
					
				VServ.bad_config = true;
			}
		}

	#pragma endregion

#pragma endregion

#pragma region Parser

	void Settings::parser(std::ifstream & infile) {
		std::string line; std::string oline; int last_line_count = -1; line_count = 0; VServer * PVServ = NULL; VServer VServ; Location Loc; Method Met;
		bool is_http = false; int section = 0, subsection = 0; int section_bracket_lvl[4] = {0, 0, 0, 0};

		while (getline(infile, line)) {
			bool NoAdd = false; std::string temp; oline = line; ++line_count; global.config.push_back(oline);
			Utils::trim(line); if (line.empty()) { if (section != ROOT && section != GLOBAL && subsection != GLOBAL) VServ.config.push_back(oline); continue; }

			while (!line.empty()) { int bracket_mode = 0;
				size_t pos = line.find_first_of("{};");
				if (pos != std::string::npos) {
					temp = line.substr(0, pos + 1);
					line = line.substr(pos + 1);
				} else {
					temp = line; line.clear(); if (temp.empty()) continue;
					if (temp.find("http") != 0 && temp.find("server") != 0 && temp.find("location") != 0 && temp.find("method") != 0) {
						std::string space = ""; if (line_count - 1 < 10) space = " ";
						std::string n_line = "[" Y + Utils::ltos(line_count - 1) + RD "] " + space;
						log_servers(RD + n_line + "Missing '" Y ";" RD "' at the end of the statement" NC, PVServ);
						if (section > GLOBAL) VServ.bad_config = true; else Settings::global.bad_config = true;
						continue;
					}
				}

				Utils::trim(line);
				if (temp[temp.size() - 1] == ';') temp.erase(temp.size() - 1);
				if (temp[temp.size() - 1] == '{') { bracket_mode = 1; temp.erase(temp.size() - 1); }
				if (temp[temp.size() - 1] == '}') { bracket_mode = -1; temp.erase(temp.size() - 1); }
				Utils::trim(temp);

				std::string firstPart, secondPart; std::istringstream stream(temp);
				stream >> firstPart; std::getline(stream, secondPart);
				Utils::trim(firstPart); Utils::toLower(firstPart); Utils::trim(secondPart);

				if (firstPart == "http" && section == ROOT && is_http == false) { section = GLOBAL; section_bracket_lvl[0] = bracket_lvl; clear(); }
				if (firstPart == "server" && section == GLOBAL) { section = SERVER; section_bracket_lvl[1] = bracket_lvl; PVServ = &VServ; }
				if (firstPart == "location" && section == SERVER) { section = LOCATION; section_bracket_lvl[2] = bracket_lvl; Loc.clear(); Loc.VServ = &VServ; }
				if (firstPart == "method" && section > ROOT && section < METHOD) { subsection = section; section = METHOD; section_bracket_lvl[3] = bracket_lvl; Met.clear();
					if (section == GLOBAL) Met.VServ = &Settings::global;
					if (section == SERVER) Met.VServ = &VServ;
					if (section == LOCATION) { Met.VServ = &VServ; Met.Loc = &Loc; }
				}

				if (section != ROOT && section != GLOBAL && subsection != GLOBAL && last_line_count != line_count) { last_line_count = line_count; VServ.config.push_back(oline); }

				if (firstPart == "http" && is_http == false) is_http = true;
				else if (firstPart == "http" && is_http == true) { invalid_directive(firstPart, line_count, ROOT, PVServ);					NoAdd = true; }
				else if (section == GLOBAL && repeated_directive(firstPart, global.data, line_count, PVServ)) 								NoAdd = true;
				else if (section == SERVER && repeated_directive(firstPart, VServ.data, line_count, PVServ)) 								NoAdd = true;
				else if (section == LOCATION && repeated_directive(firstPart, Loc.data, line_count, PVServ)) 								NoAdd = true;
				else if (section == METHOD && repeated_directive(firstPart, Met.data, line_count, PVServ)) 									NoAdd = true;
				else if (invalid_directive(firstPart, line_count, section, PVServ))	{														NoAdd = true; firstPart = ""; if (section > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }

				if ((section == GLOBAL || section == SERVER || section == LOCATION) && !firstPart.empty()) {
					if (!NoAdd && (firstPart == "access_log" || firstPart == "error_log") && parse_path(firstPart, secondPart, true, true, false, PVServ))	firstPart = "";
					if (!NoAdd && firstPart == "log_rotatesize" && parse_log_rotatesize(secondPart, PVServ))												firstPart = ""; 
					if (!NoAdd && firstPart == "log_rotate" && parse_log_rotate(secondPart, PVServ))														firstPart = ""; 
					if (!NoAdd && firstPart == "root" && parse_path(firstPart, secondPart, false, false, false, PVServ)) {					firstPart = ""; if (section > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
					if (!NoAdd && firstPart == "uploads" && parse_path(firstPart, secondPart, false, false, false, PVServ)) {				firstPart = ""; if (section > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
					if (!NoAdd && firstPart == "body_maxsize" && parse_body_size(secondPart, PVServ)) {										firstPart = ""; if (section > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
					if (!NoAdd && firstPart == "autoindex" && parse_autoindex(secondPart, PVServ)) {										firstPart = ""; if (section > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
					if (!NoAdd && firstPart == "index" && parse_index(secondPart, PVServ)) {												firstPart = ""; if (section > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
					if (!NoAdd && firstPart == "allow" && parse_allow(secondPart, PVServ)) {												firstPart = ""; if (section > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
					if (!NoAdd && firstPart == "deny" && parse_deny(secondPart, PVServ)) {													firstPart = ""; if (section > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
					if (!NoAdd && firstPart == "return" && parse_return(secondPart, PVServ)) {												firstPart = ""; if (section > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
				}

				if (section == GLOBAL && !firstPart.empty()) {
					if (!NoAdd && firstPart == "keepalive_timeout" && parse_keepalive_timeout(secondPart, PVServ))							{ firstPart = ""; global.bad_config = true; }
					if (!NoAdd && firstPart == "keepalive_requests" && parse_keepalive_requests(secondPart, PVServ))						{ firstPart = ""; global.bad_config = true; }
					if (!NoAdd && firstPart == "error_page") parse_errors(firstPart, secondPart, PVServ);
					if (!NoAdd && Utils::strToLower(firstPart) == "cgi") parse_cgi(firstPart, secondPart, PVServ);

					if (!NoAdd && (firstPart == "allow" || firstPart == "deny"))															global.add(firstPart, secondPart, true);
					else if (!NoAdd && !firstPart.empty() && firstPart != "http" && firstPart != "error_page"
						&& Utils::strToLower(firstPart) != "cgi")																			global.add(firstPart, secondPart);
				}

				if (section == SERVER && !firstPart.empty()) {
					if (!NoAdd && firstPart == "listen" && parse_listen(secondPart, VServ))													{ firstPart = ""; VServ.bad_config = true; }
					if (!NoAdd && firstPart == "error_page") parse_errors(firstPart, secondPart, VServ);
					if (!NoAdd && Utils::strToLower(firstPart) == "cgi") parse_cgi(firstPart, secondPart, VServ);

					if (!NoAdd && (firstPart == "listen" || firstPart == "allow" || firstPart == "deny"))									VServ.add(firstPart, secondPart, true);
					else if (!NoAdd && !firstPart.empty() && firstPart != "server" && firstPart != "error_page"
						&& Utils::strToLower(firstPart) != "cgi")																			VServ.add(firstPart, secondPart);
				}

				if (section == LOCATION && !firstPart.empty()) {
					if (!NoAdd && firstPart == "location" && parse_location(secondPart, PVServ))											{ firstPart = ""; VServ.bad_config = true; }
					if (!NoAdd && firstPart == "alias" && parse_alias(firstPart, secondPart, PVServ))										{ firstPart = ""; VServ.bad_config = true; }
					if (!NoAdd && firstPart == "try_files" && parse_try_files(secondPart, PVServ))											{ firstPart = ""; VServ.bad_config = true; }
					if (!NoAdd && firstPart == "internal")secondPart = "true";
					if (!NoAdd && firstPart == "error_page") parse_errors(firstPart, secondPart, Loc);
					if (!NoAdd && Utils::strToLower(firstPart) == "cgi") parse_cgi(firstPart, secondPart, VServ);

					if (!NoAdd && (firstPart == "allow" || firstPart == "deny"))															Loc.add(firstPart, secondPart, true);
					else if (!NoAdd && !firstPart.empty() && firstPart != "error_page" && Utils::strToLower(firstPart) != "cgi")			Loc.add(firstPart, secondPart);
				}

				if (section == METHOD && !firstPart.empty()) {
					if (!NoAdd && firstPart == "method" && parse_method(secondPart, PVServ))												{ firstPart = ""; if (subsection > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
					if (!NoAdd && firstPart == "allow" && parse_allow(secondPart, PVServ))													{ firstPart = ""; if (subsection > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
					if (!NoAdd && firstPart == "deny" && parse_deny(secondPart, PVServ))													{ firstPart = ""; if (subsection > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }
					if (!NoAdd && firstPart == "return" && parse_return(secondPart, PVServ))												{ firstPart = ""; if (subsection > GLOBAL) VServ.bad_config = true; else global.bad_config = true; }

					if (!NoAdd && (firstPart == "allow" || firstPart == "deny"))															Met.add(firstPart, secondPart, true);
					else if (!firstPart.empty())	 																						Met.add(firstPart, secondPart);
				}

				if (bracket_mode != 0) { bracket_lvl += bracket_mode;
					if (section == METHOD && section_bracket_lvl[3] == bracket_lvl)   {
						section = subsection; subsection = 0; section_bracket_lvl[3] = 0;
						if (section == GLOBAL) {
							Met.VServ = &global; global.add(Met);
							global.add("method", Utils::ltos(global.method.size() - 1), true);
						}
						if (section == SERVER) {
							VServ.add(Met); VServ.add("method", Utils::ltos(VServ.method.size() - 1), true);
						}
						if (section == LOCATION) {
							Loc.add(Met); Loc.add("method", Utils::ltos(Loc.method.size() - 1), true);
						}
					}

					if (section == LOCATION && section_bracket_lvl[2] == bracket_lvl) {
						section = SERVER; section_bracket_lvl[2] = 0;
						VServ.add(Loc); VServ.add("location", Utils::ltos(VServ.location.size() - 1), true);
					}

					if (section == SERVER && section_bracket_lvl[1] == bracket_lvl)   {
						section = GLOBAL; section_bracket_lvl[1] = 0; missing_directive(VServ); if (VServ.bad_config) VServ.force_off = true;
						add(VServ); global.add("server", Utils::ltos(Settings::vserver.size() - 1), true);
						VServ.bad_config = false; VServ.force_off = false; VServ.clear(); PVServ = NULL;

						for (std::deque<Location>::iterator l_it = vserver[vserver.size() - 1].location.begin(); l_it != vserver[vserver.size() - 1].location.end(); ++l_it) {
							for (std::deque<Method>::iterator m_it =  l_it->method.begin(); m_it != l_it->method.end(); ++m_it)  m_it->Loc = &(*l_it);
							l_it->VServ = &vserver[vserver.size() - 1];
						}
					}

					if (section == GLOBAL && section_bracket_lvl[0] == bracket_lvl)   { section = ROOT; section_bracket_lvl[0] = 0; }
				}
			}
		}
		if		(bracket_lvl != 0)    		{								global.bad_config = true; log_error_add(RD "Brackets error" NC); }
		else if	(vserver.size() == 0)		{								global.bad_config = true; log_access_add("---"); log_error_add(RD "There are no " Y "virtual servers" RD " in the configuration file" NC); }
		else {
			bool valid_vserver = false;
			for (std::deque<VServer>::iterator it = vserver.begin(); it != vserver.end(); ++it)
				if (it->bad_config == false) { valid_vserver = true; break; }
			if (valid_vserver == false && vserver.size() > 0) {		global.bad_config = true; log_access_add("---"); log_error_add(RD "There are no " Y "virtual servers" RD " available" NC); }
		}
	}

#pragma endregion
