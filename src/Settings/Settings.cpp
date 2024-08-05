/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:27:58 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/05 23:09:53 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"

#pragma region Variables

	std::map <std::string, std::string>	Settings::global;
	std::vector <VServer> 				Settings::vserver;
	std::string							Settings::program_path = Settings::programPath();
	std::string							Settings::config_path = Settings::programPath() + "conf/";
	std::vector <std::string>			Settings::config;
	size_t								Settings::config_index = 0;										//	Current index of the settings
	size_t								Settings::log_index = 0;										//	Current index of main log
	bool								Settings::config_displayed = false;								//	Is the log or the settings displayed
	bool								Settings::autolog = true;										//	Auto scroll logs
	int 								Settings::terminate = -1;
	int									Settings::bracket_lvl = 0;
	bool 								Settings::check_only = false;
	bool 								Settings::loaded_ok = false;
	bool 								Settings::errors = false;
	bool 								Settings::status = false;
	Timer 								Settings::timer;
	int									Settings::current_vserver = -1;

	static int							line_count = 0;

#pragma endregion

#pragma region Utils

	#pragma region Program Path

		std::string Settings::programPath() {
			char result[PATH_MAX];
			ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
			if (count != -1) return (std::string(result, count - 7));
			return ("/");
		}

	#pragma endregion

	#pragma region Create Path

		int Settings::createPath(const std::string & path) {
			size_t pos = 0; std::string dir;

			if (path == "") return (0);
			while ((pos = path.find('/', pos)) != std::string::npos) {
				dir = path.substr(0, pos++);
				if (dir.empty()) continue;
				if (mkdir(dir.c_str(), 0755) == -1 && errno != EEXIST) return (1);
			} return (0);
		}

	#pragma endregion

	#pragma region File Exists

		static int file_exists(const std::string & File) {
			if (access(File.c_str(), F_OK) < 0) return (1);
			if (access(File.c_str(), R_OK) < 0) return (2);
			return (0);
		}

	#pragma endregion

	#pragma region Generate Config

		static void generate_config(const std::string & File) {
			Settings::createPath(File); std::ofstream outfile;
			outfile.open(File.c_str(), std::ios_base::app);
			if (outfile.is_open()) {
				outfile << "access_log logs/access.log;" << std::endl
						<< "error_log logs/error.log;" << std::endl << std::endl
						<< "client_max_body_size 10M;" << std::endl << std::endl
						<< "error_page 404 /404.html;" << std::endl
						<< "error_page 500 502 503 504 /50x.html;" << std::endl << std::endl
						<< "autoindex on;" << std::endl << std::endl
						<< "server {" << std::endl
						<< "    listen 80;" << std::endl
						<< "    root " << Settings::program_path + "www/html" << ";" << std::endl
						<< "    index index.html;" << std::endl
						<< "    server_name default;" << std::endl << std::endl
						<< "    location / {" << std::endl
						<< "        try_files $uri $uri/ =404;" << std::endl
						<< "    }" << std::endl
						<< "}" << std::endl;
				outfile.close();
				Settings::createPath(Settings::program_path + "www/html");
				Log::log_error("Default configuration file created succesfully", NULL, true);
			}
		}

	#pragma endregion

	#pragma region Trim

		static void trim(std::string &str) {
			std::string::iterator start = str.begin();
			std::string::iterator end = str.end();

			while (start != str.end() && std::isspace(static_cast<unsigned char>(*start))) start++;

			std::string::iterator hashPos = std::find(start, str.end(), '#');
			if (hashPos != str.end()) end = hashPos;

			while (end != start && std::isspace(static_cast<unsigned char>(*(end - 1)))) --end;
			str = std::string(start, end);
		}

	#pragma endregion

	#pragma region toLower

		static void toLower(std::string & str) {
			for (size_t i = 0; i < str.size(); ++i) str[i] = std::tolower(static_cast<unsigned char>(str[i]));
		}

	#pragma endregion

	#pragma region Print

	void Settings::print() {
		// Imprimir Settings::global
		std::cout << "Global Settings:" << std::endl << std::endl;
		for (std::map<std::string, std::string>::iterator it = Settings::global.begin(); it != Settings::global.end(); ++it) {
			std::cout << it->first << ": " << it->second << std::endl;
		}
		std::cout << std::endl;

		// Imprimir cada VServer
		for (size_t i = 0; i < Settings::vserver.size(); ++i) {
			const VServer& vserver = Settings::vserver[i];
			std::cout << "VServer " << i + 1 << ":" << std::endl << std::endl;
			for (std::map<std::string, std::string>::const_iterator it = vserver.vserver.begin(); it != vserver.vserver.end(); ++it) {
				std::cout << it->first << ": " << it->second << std::endl;
			}
			std::cout << std::endl;

			// Imprimir cada Location dentro del VServer
			for (size_t j = 0; j < vserver.location.size(); ++j) {
				const Location& location = vserver.location[j];
				std::cout << "  Location " << j + 1 << ":" << std::endl << std::endl;
				for (std::map<std::string, std::string>::const_iterator it = location.location.begin(); it != location.location.end(); ++it) {
					std::cout << "    " << it->first << ": " << it->second << std::endl;
				}
				std::cout << std::endl;
			}
		}
	}

	#pragma endregion

#pragma endregion

#pragma region Parser

	#pragma region Brackets

		static int brackets(std::string & str) {
			int change = 0;
			for (size_t i = 0; i < str.size(); ++i) {
				if (str[i] == '{') {
					str.erase(i, 1); --i;										//	Delete '{' and adjust index
					Settings::bracket_lvl++;
					change += 1;
				} else if (str[i] == '}') {
					str.erase(i, 1); --i;										//	Delete '}' and adjust index
					Settings::bracket_lvl--;
					change -= 1;
				}
			}
			return (change);
		}

	#pragma endregion

	#pragma region Parse Body Size

		static int parse_body_size(std::string & str, std::string & line) {
			std::stringstream result; long multiplier = 1;

			if (str.empty()) { Log::log_error("Empty value for client_max_body_size - '" + line + "'", NULL, true); return (1); }

			if (!std::isdigit(str[str.size() - 1])) {
				result.str(str.substr(0, str.size() - 1));
				switch (str[str.size() - 1]) {
					case 'K': multiplier = 1024; break;
					case 'M': multiplier = 1024 * 1024; break;
					case 'G': multiplier = 1024 * 1024 * 1024; break;
				}
			}

			std::stringstream ss(result.str());

			std::stringstream ss_error;
		    ss_error << "Invalid value for client_max_body_size - " << line_count << " '" << line << "'";

			long number; ss >> number; if (ss.fail() || !ss.eof()) { Log::log_error(ss_error.str(), NULL, true); return (1); }
			if (number < 1) { Log::log_error("value for client_max_body_size cannot be lower than 1 byte - '" + line + "'", NULL, true); return (1); }
			if (number > 1024 * 1024 * 1024) { Log::log_error("Value for client_max_body_size cannot be greater than 1GB - '" + line + "'", NULL, true); return (1); }
			
			result << number * multiplier; str = result.str(); if (result.fail()) { Log::log_error("Invalid value for client_max_body_size - '" + line + "'", NULL, true); return (1); }
			return (0);
		}

	#pragma endregion

	#pragma region Parse Error Codes

		static void parse_errors(const std::string & firstPart, const std::string & secondPart) {
			std::istringstream stream(secondPart);
			std::vector<std::string> errors;
			std::string error;

			while (stream >> error) errors.push_back(error);
			std::string filePath = errors.back(); errors.pop_back();

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				Settings::add(firstPart + " " + *it, filePath);
			}
		}

		static void parse_errors(const std::string & firstPart, const std::string & secondPart, VServer & VServ) {
			std::istringstream stream(secondPart);
			std::vector<std::string> errors;
			std::string error;

			while (stream >> error) errors.push_back(error);
			std::string filePath = errors.back(); errors.pop_back();

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				VServ.add(firstPart + " " + *it, filePath);
			}
		}

		static void parse_errors(const std::string & firstPart, const std::string & secondPart, Location & Loc) {
			std::istringstream stream(secondPart);
			std::vector<std::string> errors;
			std::string error;

			while (stream >> error) errors.push_back(error);
			std::string filePath = errors.back(); errors.pop_back();

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				Loc.add(firstPart + " " + *it, filePath);
			}
		}

	#pragma endregion

	#pragma region Parse Location

		static int parser_location(std::ifstream & infile, std::string & line, VServer & VServ) {
			bool inLimit = false; Location Loc; int current_bracket = Settings::bracket_lvl; std::string tmp_line; std::string orig_line = line;
			do {
				line_count++; tmp_line = line;
				trim(line); if (line.empty()) {
					VServ.config.push_back(line);
					Settings::config.push_back(line);
					continue;
				}
				std::string firstPart, secondPart; std::istringstream stream(line);

				stream >> firstPart; trim(firstPart); toLower(firstPart);

				std::getline(stream, secondPart); trim(secondPart);

				if (tmp_line != orig_line) { VServ.config.push_back(tmp_line); Settings::config.push_back(tmp_line); }
				if (!firstPart.empty() && firstPart[firstPart.size() - 1] == ';') firstPart.erase(firstPart.size() - 1);
				if (!secondPart.empty() && secondPart[secondPart.size() - 1] != '{' && secondPart[secondPart.size() - 1] != '}' && secondPart[secondPart.size() - 1] != ';' && firstPart != "{" && firstPart != "}" && firstPart != "location") return (0);
				if (!secondPart.empty() && secondPart[secondPart.size() - 1] == ';') secondPart.erase(secondPart.size() - 1);

				if (brackets(firstPart) + brackets(secondPart) < 0) {
					if (inLimit) inLimit = false;
					if (Settings::bracket_lvl < 0) return (0);
					if (Settings::bracket_lvl <= current_bracket) { VServ.add(Loc); break; }
				}

				if (firstPart.empty()) continue;
				if (firstPart == "limit_except") inLimit = true;
				if (firstPart == "client_max_body_size") parse_body_size(secondPart, line);
				if (firstPart == "error_page") parse_errors(firstPart, secondPart, Loc); else Loc.add(firstPart, secondPart);
				
			} while (getline(infile, line));
			return (0);
		}

	#pragma endregion

	#pragma region Parse VServer

		static int parser_vserver(std::ifstream & infile, std::string & line) {
			VServer VServ; int current_bracket = Settings::bracket_lvl; std::string tmp_line; std::string orig_line = line;
			do {
				tmp_line = line;
				trim(line); if (line.empty()) {
					VServ.config.push_back(tmp_line);
					Settings::config.push_back(tmp_line);
					line_count++; continue;
				}
				std::string firstPart, secondPart; std::istringstream stream(line);
				
				stream >> firstPart; trim(firstPart); toLower(firstPart);

				if (firstPart == "location") {
					VServ.config.push_back(tmp_line); Settings::config.push_back(tmp_line);
					parser_location(infile, line, VServ);
					firstPart = "";
				}
				
				if (!firstPart.empty()) {
					line_count++;
					std::getline(stream, secondPart); trim(secondPart);
					if (!firstPart.empty() && firstPart[firstPart.size() - 1] == ';') firstPart.erase(firstPart.size() - 1);
					if (!secondPart.empty() && secondPart[secondPart.size() - 1] != '{' && secondPart[secondPart.size() - 1] != '}' && secondPart[secondPart.size() - 1] != ';' && firstPart != "{" && firstPart != "}" && firstPart != "location") return (0);
					if (!secondPart.empty() && secondPart[secondPart.size() - 1] == ';') secondPart.erase(secondPart.size() - 1);
				}

				if (brackets(firstPart) + brackets(secondPart) < 0) {
					if (Settings::bracket_lvl < 0) return (0);
					if (Settings::bracket_lvl <= current_bracket) { Settings::config.push_back(tmp_line); Settings::add(VServ); break; }
				}

				if (!firstPart.empty() && tmp_line != orig_line) { VServ.config.push_back(tmp_line); Settings::config.push_back(tmp_line); }
				if (firstPart.empty()) continue;
				if (firstPart == "client_max_body_size") parse_body_size(secondPart, line);
				if (firstPart == "error_page") parse_errors(firstPart, secondPart, VServ);
				else if (firstPart != "server") VServ.add(firstPart, secondPart);

			} while (getline(infile, line));
			return (0);
		}

	#pragma endregion

	#pragma region Parse Line

		static int parse_line(std::ifstream & infile, std::string & line) {
			std::string firstPart, secondPart;
			Settings::config.push_back(line);
			trim(line); if (line.empty()) { line_count++; return (0); }
			std::istringstream stream(line);

			stream >> firstPart; trim(firstPart); toLower(firstPart);

			if (firstPart == "server") {
				parser_vserver(infile, line);
				firstPart = "";
			}

			if (!firstPart.empty()) {
				line_count++;
				std::getline(stream, secondPart); trim(secondPart);
				if (!firstPart.empty() && firstPart[firstPart.size() - 1] == ';') firstPart.erase(firstPart.size() - 1);
				if (!secondPart.empty() && secondPart[secondPart.size() - 1] != '{' && secondPart[secondPart.size() - 1] != '}' && secondPart[secondPart.size() - 1] != ';' && firstPart != "{" && firstPart != "}" && firstPart != "server" && firstPart != "location") return (0);
				if (!secondPart.empty() && secondPart[secondPart.size() - 1] == ';') secondPart.erase(secondPart.size() - 1);
			}

			brackets(firstPart); brackets(secondPart);

			if (firstPart.empty()) return (0);
			if (firstPart == "client_max_body_size") parse_body_size(secondPart, line);
			if (firstPart == "error_page") parse_errors(firstPart, secondPart); else Settings::add(firstPart, secondPart);

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region Load

	#pragma region Load File

		void Settings::load(const std::string & File, bool isRegen) {
			bool isDefault = (File == Settings::config_path + "default.cfg");
			std::string	line; std::ifstream infile(File.c_str());
			Settings::clear();

			if (infile.is_open()) {
				while (getline(infile, line)) {
					if (parse_line(infile, line)) {
						infile.close();
						if (isDefault) {
							if (!isRegen) {
								remove(File.c_str());
								Settings::clear();
								Log::log_error("Default configuration file is corrupted, generating a default config file", NULL, true);
								generate_config(File);
								Settings::load(File, true);
								return ;
							} else {
								Log::log_error("Could not create the default configuration file", NULL, true);
							}
						} else {
							Log::log_error("Could not load the configuration file '" + File + "'");
						}
						Settings::terminate = 1;
						return ;
					}
				} infile.close();
				if (Settings::bracket_lvl != 0) {
				// 	if (isDefault) {
				// 		if (!isRegen) {
				// 			remove(File.c_str());
				// 			Settings::clear();
				// 			Log::log_error("Default configuration file is corrupted, generating a default config file", NULL, true);
				// 			generate_config(File);
				// 			Settings::load(File, true);
				// 			return ;
				// 		} else {
				// 			Log::log_error("Could not create the default configuration file", NULL, true);
				// 		}
				// 	} else {
				// 		Log::log_error("Could not load the configuration file '" + File + "'");
				// 	}
					Settings::terminate = 1;
				}

				Settings::loaded_ok = true;
				if (isDefault)
					Log::log_access("Default configuration file loaded succesfully");
				else
					Log::log_access("Configuration file '" + File + "' loaded succesfully");

			} else {
				if (isDefault) {
					Log::log_error("Could not create the default configuration file", NULL, true);
				} else {
					if (file_exists(File) == 1)
						Log::log_error("The configuration file '" + File + "' does not exist", NULL, true);
					else if (file_exists(File) == 2)
						Log::log_error("Cannot read the file '" + File + "'", NULL, true);
					else
						Log::log_error("Could not load the configuration file '" + File + "'", NULL, true);
				}
			}
		}

	#pragma endregion

	#pragma region Load Default

		void Settings::load() {
			std::string File = Settings::config_path + "default.cfg";
			int FileStatus = file_exists(File);

			if (FileStatus) {
				if (FileStatus == 1)
					Log::log_error("Default configuration file does not exist, generating a default config file", NULL, true);
				else if (FileStatus == 2) {
					Log::log_error("Cannot read the default configuration file, generating a default config file", NULL, true);
					remove(File.c_str());
				} else {
					Log::log_error("Could not load the default configuration file, generating a default config file", NULL, true);
					remove(File.c_str());
				}
				generate_config(File);
			}
			Settings::load(File);
		}

#pragma endregion

	#pragma region Load Args

		void Settings::load_args(int argc, char **argv) {
			if (argc == 2 && !strcmp(argv[1], "-t")) {                                                                                      //  Test default settings
				Settings::check_only = true;
				Settings::load();
				Settings::terminate = 0;
			} else if (argc == 3 && (!strcmp(argv[1], "-t") || !strcmp(argv[2], "-t"))) {                                                   //  Test indicated settings
				Settings::check_only = true;
				if (!strcmp(argv[1], "-t") && !strcmp(argv[2], "-t")) {
					std::cout << std::endl << RD "\t\t     Test config file" << std::endl << std::endl
							<< C "\tUsage: " Y "./webserv -t [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
				}
				else if (strcmp(argv[1], "-t")) Settings::load(argv[1]);
				else if (strcmp(argv[2], "-t")) Settings::load(argv[2]);
				Settings::terminate = 0;
			} else if (argc > 2) {
				std::cout << std::endl << RD "\t     Incorrect number of arguments" << std::endl << std::endl
						<< C "\tUsage: " Y "./webserv [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
				Settings::terminate = 1;
			} else {
				if (argc == 1) Settings::load(); else Settings::load(argv[1]);
				if (Settings::vserver.size() == 0) {
					if (Settings::loaded_ok) Log::log_error("There are no virtual servers in the configuration file", NULL, true);
				// 	std::cout << std::endl << C "\tCould not start the server, check the file:" << std::endl << std::endl
				// 			<< Y "\t" << Settings::program_path + "logs/error.log" NC << std::endl << std::endl;
				// 	Settings::terminate = 1;
				}
			}
		}

#pragma endregion

#pragma endregion

#pragma region Global

	std::string Settings::get(const std::string & Key) {
		std::map<std::string, std::string>::const_iterator it = global.find(Key);
		if (it == global.end()) return ("");
		return (it->second);
	}

	void Settings::set(const std::string & Key, const std::string & Value) {
		std::map<std::string, std::string>::iterator it = global.find(Key);
		if (it != global.end()) it->second = Value;
		if (it == global.end()) global[Key] = Value;
	}

	void Settings::del(const std::string & Key) {
		std::map<std::string, std::string>::iterator it = global.find(Key);
		if (it != global.end()) global.erase(it);
	}

	void Settings::clear() {
		for (std::vector<VServer>::iterator it = vserver.begin(); it != vserver.end(); ++it) it->clear();
		global.clear(); vserver.clear(); config.clear(); Settings::bracket_lvl = 0; Settings::loaded_ok = false;
	}

#pragma endregion

#pragma region VServer

	void Settings::set(VServer & VServ) {
		std::vector<VServer>::iterator it = std::find(vserver.begin(), vserver.end(), VServ);
		if (it == vserver.end()) { VServ.status = false; VServ.config_displayed = false; VServ.log_index = 0; VServ.config_index = 0; VServ.autolog = true; vserver.push_back(VServ); }
		else *it = VServ;
	}

	void Settings::del(const VServer & VServ) {
		std::vector<VServer>::iterator it = std::find(vserver.begin(), vserver.end(), VServ);
		if (it != vserver.end()) { it->clear(); vserver.erase(it); }
	}

#pragma endregion
