/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:27:58 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 15:39:42 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"

#pragma region Variables
	Timer 								Settings::timer;												//	Class to obtain time and date related data
	std::string							Settings::program_path = Settings::programPath();				//	Path of the executable
	std::string							Settings::config_path = Settings::programPath() + "conf/";		//	Path of the default configuration file
	std::map <int, std::string>			Settings::error_codes;											//	Error codes in a map
	std::map <std::string, std::string>	Settings::global;												//	Global settings in a map
	std::vector <VServer> 				Settings::vserver;												//	V-Servers in a vector
	std::vector <std::string>			Settings::config;												//	Configuration file in a vector
	bool								Settings::config_displayed = false;								//	Is the log or the settings displayed
	size_t								Settings::config_index = 0;										//	Current index of the settings
	size_t								Settings::log_index = 0;										//	Current index of the main log
	bool								Settings::autolog = true;										//	Auto-Scroll logs
	bool 								Settings::check_only = false;									//	Check the config file, but don't start the server
	bool 								Settings::loaded_ok = false;									//	The config file loaded successfully (but may contains errors)
	bool 								Settings::status = false;										//	Status of the server (On/Off)
	bool 								Settings::BadConfig = false;									//	Indicate if there are errors in the config file
	int									Settings::current_vserver = -1;									//	Current selected V-Server (-1 = None)
	int 								Settings::terminate = -1;										//	Flag the program to exit with the value in terminate (the default value of -1 don't exit)

	static int							line_count = 0;													//	Number of the current line of the configuration file (use to indicate the line of an error in the configuration file)
	static int							bracket_lvl = 0;												//	Level of the bracket (use to parse the configuration file)

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
				Log::log_error("Default configuration file created succesfully");
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

#pragma endregion

#pragma region Parser

	std::string Settings::itos(long number) {
		std::stringstream ss; ss << number;
		if (ss.fail()) return ("");
		return (ss.str());
	}

	bool stol(const std::string & str, long & number) {
		std::stringstream ss(str);
		ss >> number; return ((ss.fail() || !ss.eof()));
	}

	#pragma region Brackets

		static int brackets(std::string & str) {
			int change = 0;
			for (size_t i = 0; i < str.size(); ++i) {
				if (str[i] == '{') {
					str.erase(i, 1); --i;										//	Delete '{' and adjust index
					bracket_lvl++;
					change += 1;
				} else if (str[i] == '}') {
					str.erase(i, 1); --i;										//	Delete '}' and adjust index
					bracket_lvl--;
					change -= 1;
				}
			}
			return (change);
		}

	#pragma endregion

	#pragma region Parse Path

		static int parse_path(std::string & firstPart, std::string & str, bool isFile = false, bool check_path = false) {
			std::string n_line = "[" Y + Settings::itos(line_count - 1) + RD "] "; struct stat info;

			if (str.empty())										Log::log_error(n_line + "Empty value for '" Y + firstPart + RD "'" NC);
			if (str[0] != '/') str = Settings::program_path + str;

			if (isFile && check_path) {
				std::string dir_path = str.substr(0, str.find_last_of('/'));
				if (stat(dir_path.c_str(), &info) != 0) {
					if (errno == ENOENT)							Log::log_error(n_line + "The path '" Y + dir_path + RD "' does not exist" NC);
					else if (errno == EACCES)						Log::log_error(n_line + "No permission to access '" Y + dir_path + RD "'" NC);
					else											Log::log_error(n_line + "Cannot access '" Y + dir_path + RD "'" NC);
					return (1);
				} else {
					if (!(info.st_mode & S_IFDIR)) {				Log::log_error(n_line + "'" Y + dir_path + RD "' is not a valid directory" NC); return (1); }
					else if (access(dir_path.c_str(), W_OK) != 0) {	Log::log_error(n_line + "No write permission for '" Y + dir_path + RD "'" NC); return (1); }
					else if (access(str.c_str(), F_OK) == 0
						&& access(str.c_str(), W_OK) != 0) {		Log::log_error(n_line + "No write permission for '" Y + str + RD "'" NC); return (1); }
				}
			} else if (stat(str.c_str(), &info) != 0) {
				if (errno == ENOENT)								Log::log_error(n_line + "The path '" Y + str + RD "' does not exist" NC);
				else if (errno == EACCES)							Log::log_error(n_line + "No permission to access '" Y + str + RD "'" NC);
				else 												Log::log_error(n_line + "Cannot access '" Y + str + RD "'" NC);
				return (1);
			} else {
				if (isFile && !(info.st_mode & S_IFREG)) { 			Log::log_error(n_line + "'" Y + str + RD "' is not a valid file" NC); return (1); return (1); }
				else if (!isFile && !(info.st_mode & S_IFDIR)) { 	Log::log_error(n_line + "'" Y + str + RD "' is not a valid directory" NC); return (1); }
			}
			return (0);
		}

	#pragma endregion

	#pragma region Parse Body Size

		static int parse_body_size(std::string & str) {
			long multiplier = 1; std::string n_line = "[" Y + Settings::itos(line_count - 1) + RD "] ";

			if (str.empty()) { Log::log_error(n_line + "Empty value for '" Y "client_max_body_size" RD "'" NC); return (1); }
			if (str.size() > 1 && !std::isdigit(str[str.size() - 1]) && !std::isdigit(str[str.size() - 2]) && str[str.size() - 1] == 'B') str.erase(str.size() - 1);
			if (!std::isdigit(str[str.size() - 1])) {
				switch (str[str.size() - 1]) {
					case 'K': multiplier = 1024; break;
					case 'M': multiplier = 1024 * 1024; break;
					case 'G': multiplier = 1024 * 1024 * 1024; break;
					case 'B' : break;
					default : { Log::log_error(n_line + "Invalid value for '" Y "client_max_body_size" RD "'" NC); return (1); }
				} str.erase(str.size() - 1);
			}

			long number; if (stol(str, number) || (str = Settings::itos(number * multiplier)) == "") { Log::log_error(n_line + "Invalid value for '" Y "client_max_body_size" RD "'" NC); return (1); }
			if (number < 1) { Log::log_error(n_line + "Value for '" Y "client_max_body_size" RD "' cannot be lower than 1 byte" NC); return (1); }
			if (number > 1024 * 1024 * 1024) { Log::log_error(n_line + "Value for '" Y "client_max_body_size" RD "' cannot be greater than 1GB" NC); return (1); }
			return (0);
		}

	#pragma endregion

	#pragma region Parse Error Codes

		static int parse_errors(const std::string & firstPart, const std::string & secondPart) {
			std::string n_line = "[" Y + Settings::itos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart);
			std::vector<std::string> errors;
			std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { Log::log_error(n_line + "Empty value for '" Y "error_page" RD "'" NC); return (1); }
			std::string filePath = errors.back(); errors.pop_back();
			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (stol(*it, code) || (Settings::error_codes.find(code) == Settings::error_codes.end())) {
					Log::log_error(n_line + "Invalid error number '" Y +  *it + RD "' for '" Y "error_page" RD "'" NC); Settings::BadConfig = true;
				}
				Settings::add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

		static int parse_errors(const std::string & firstPart, const std::string & secondPart, VServer & VServ) {
			std::string n_line = "[" Y + Settings::itos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart);
			std::vector<std::string> errors;
			std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { Log::log_error(n_line + "Empty value for '" Y "error_page" RD "'" NC); return (1); }
			std::string filePath = errors.back(); errors.pop_back();

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (stol(*it, code) || (Settings::error_codes.find(code) == Settings::error_codes.end())) {
					Log::log_error(n_line + "Invalid error number '" Y +  *it + RD "' for '" Y "error_page" RD "'" NC); Settings::BadConfig = true;
				}
				VServ.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

		static int parse_errors(const std::string & firstPart, const std::string & secondPart, Location & Loc) {
			std::string n_line = "[" Y + Settings::itos(line_count - 1) + RD "] ";
			std::istringstream stream(secondPart);
			std::vector<std::string> errors;
			std::string error;

			while (stream >> error) errors.push_back(error);
			if (errors.size() < 2) { Log::log_error(n_line + "Empty value for '" Y "error_page" RD "'" NC); return (1); }
			std::string filePath = errors.back(); errors.pop_back();

			for (std::vector<std::string>::iterator it = errors.begin(); it != errors.end(); ++it) {
				long code; if (stol(*it, code) || (Settings::error_codes.find(code) == Settings::error_codes.end())) {
					Log::log_error(n_line + "Invalid error number '" Y +  *it + RD "' for '" Y "error_page" RD "'" NC); Settings::BadConfig = true;
				}
				Loc.add(firstPart + " " + *it, filePath);
			}
			return (0);
		}

	#pragma endregion

	#pragma region Parse Autoindex

		static int parse_autoindex(std::string & str) {
			std::string n_line = "[" Y + Settings::itos(line_count - 1) + RD "] ";

			if (str.empty()) { Log::log_error(n_line + "Empty value for " Y "'autoindex'" NC); return (1); }
			if (str != "on" && str != "off") { Log::log_error(n_line + "Invalid value for " Y "'autoindex'" NC); return (1); }
			
			return (0);
		}

	#pragma endregion

	#pragma region Parse Location

		static int parser_location(std::ifstream & infile, std::string & line, VServer & VServ) {
			bool inLimit = false; Location Loc; int current_bracket = bracket_lvl; std::string tmp_line; std::string orig_line = line;
			do {
				line_count++; tmp_line = line;
				trim(line); if (line.empty()) {
					VServ.config.push_back(tmp_line);
					Settings::config.push_back(tmp_line);
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
					if (bracket_lvl < 0) return (0);
					if (bracket_lvl <= current_bracket) { VServ.add(Loc); break; }
				}

				if (firstPart.empty()) continue;
				if (firstPart == "limit_except") inLimit = true;

				if (firstPart == "access_log" || firstPart == "error_log") parse_path(firstPart, secondPart, true, true);
				if (firstPart == "root" && parse_path(firstPart, secondPart, false, false))										Settings::BadConfig = true;
				if (firstPart == "client_max_body_size" && parse_body_size(secondPart))											Settings::BadConfig = true;
				if (firstPart == "autoindex" && parse_autoindex(secondPart))													Settings::BadConfig = true;
				if (firstPart == "error_page") parse_errors(firstPart, secondPart, Loc);
				if (firstPart != "error_page") Loc.add(firstPart, secondPart);
				
			} while (getline(infile, line));
			return (0);
		}

	#pragma endregion

	#pragma region Parse VServer

		static int parser_vserver(std::ifstream & infile, std::string & line) {
			VServer VServ; int current_bracket = bracket_lvl; std::string tmp_line; std::string orig_line = line;
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
					if (bracket_lvl < 0) return (0);
					if (bracket_lvl <= current_bracket) { Settings::config.push_back(tmp_line); Settings::add(VServ); break; }
				}

				if (!firstPart.empty() && tmp_line != orig_line) { VServ.config.push_back(tmp_line); Settings::config.push_back(tmp_line); }
				if (firstPart.empty()) continue;

				if (firstPart == "access_log" || firstPart == "error_log") parse_path(firstPart, secondPart, true, true);
				if (firstPart == "root" && parse_path(firstPart, secondPart, false, false))										Settings::BadConfig = true;
				if (firstPart == "client_max_body_size" && parse_body_size(secondPart))											Settings::BadConfig = true;
				if (firstPart == "autoindex" && parse_autoindex(secondPart))													Settings::BadConfig = true;
				if (firstPart == "error_page") parse_errors(firstPart, secondPart, VServ);
				if (firstPart != "server" && firstPart == "error_page") VServ.add(firstPart, secondPart);

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

			if (firstPart == "access_log" || firstPart == "error_log") parse_path(firstPart, secondPart, true, true);
			if (firstPart == "root" && parse_path(firstPart, secondPart, false, false))											Settings::BadConfig = true;
			if (firstPart == "client_max_body_size" && parse_body_size(secondPart))												Settings::BadConfig = true;
			if (firstPart == "autoindex" && parse_autoindex(secondPart))														Settings::BadConfig = true;
			if (firstPart == "error_page") parse_errors(firstPart, secondPart);
			if (firstPart != "error_page") Settings::add(firstPart, secondPart);

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
								Log::log_error("Default configuration file is corrupted, generating a default config file");
								generate_config(File);
								Settings::load(File, true);
								return ;
							} else {
								Log::log_error("Could not create the default configuration file");
							}
						} else {
							Log::log_error("Could not load the configuration file '" + File + "'");
						}
						Settings::terminate = 1;
						return ;
					}
				} infile.close();
				if (bracket_lvl != 0) {
				// 	if (isDefault) {
				// 		if (!isRegen) {
				// 			remove(File.c_str());
				// 			Settings::clear();
				// 			Log::log_error("Default configuration file is corrupted, generating a default config file");
				// 			generate_config(File);
				// 			Settings::load(File, true);
				// 			return ;
				// 		} else {
				// 			Log::log_error("Could not create the default configuration file");
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
					Log::log_error("Could not create the default configuration file");
				} else {
					if (file_exists(File) == 1)
						Log::log_error("The configuration file '" + File + "' does not exist");
					else if (file_exists(File) == 2)
						Log::log_error("Cannot read the file '" + File + "'");
					else
						Log::log_error("Could not load the configuration file '" + File + "'");
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
					Log::log_error("Default configuration file does not exist, generating a default config file");
				else if (FileStatus == 2) {
					Log::log_error("Cannot read the default configuration file, generating a default config file");
					remove(File.c_str());
				} else {
					Log::log_error("Could not load the default configuration file, generating a default config file");
					remove(File.c_str());
				}
				generate_config(File);
			}
			Settings::load(File);
		}

#pragma endregion

	#pragma region Load Args

		void Settings::load_args(int argc, char **argv) {
			load_error_codes();
			if (argc == 2 && !strcmp(argv[1], "-t")) {                                                                                      //  Test default settings
				Settings::check_only = true;
				std::cout << std::endl;
				Settings::load();
				if (Settings::vserver.size() == 0 && Settings::loaded_ok && Log::error.size() == 0) Log::log_error("There are no " Y "virtual servers" RD " in the configuration file");
				if (Log::error.size() == 0)			std::cout << C "\tThe configuration file is correct" NC << std::endl;
				else if (Log::error.size() == 1)	std::cout << std::endl << C "\t\tThere is "  << Log::error.size() << " error in total" NC << std::endl;
				else 								std::cout << std::endl << C "\t\tThere are " << Log::error.size() << " errors in total" NC << std::endl;
				std::cout << std::endl;
				Settings::terminate = 0;
			} else if (argc == 3 && (!strcmp(argv[1], "-t") || !strcmp(argv[2], "-t"))) {                                                   //  Test indicated settings
				Settings::check_only = true;
				std::cout << std::endl;
				if (!strcmp(argv[1], "-t") && !strcmp(argv[2], "-t")) {
					std::cout << RD "\t\t     Test config file" << std::endl << std::endl
							<< C "\tUsage: " Y "./webserv -t [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
				}
				else if (strcmp(argv[1], "-t")) Settings::load(argv[1]);
				else if (strcmp(argv[2], "-t")) Settings::load(argv[2]);
				if (Settings::vserver.size() == 0 && Settings::loaded_ok && Log::error.size() == 0) Log::log_error("There are no " Y "virtual servers" RD " in the configuration file");
				if (Log::error.size() == 0)			std::cout << C "\tThe configuration file is correct" NC << std::endl;
				else if (Log::error.size() == 1)	std::cout << std::endl << C "\t\tThere is "  << Log::error.size() << " error in total" NC << std::endl;
				else 								std::cout << std::endl << C "\t\tThere are " << Log::error.size() << " errors in total" NC << std::endl;
				std::cout << std::endl;
				Settings::terminate = 0;
			} else if (argc > 2) {
				std::cout << std::endl << RD "\t     Incorrect number of arguments" << std::endl << std::endl
						<< C "\tUsage: " Y "./webserv [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
				Settings::terminate = 1;
			} else {
				if (argc == 1) Settings::load(); else Settings::load(argv[1]);
				if (Settings::vserver.size() == 0) {
					if (Settings::loaded_ok) Log::log_error("There are no virtual servers in the configuration file");
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
		global.clear(); vserver.clear(); config.clear(); bracket_lvl = 0; Settings::loaded_ok = false;
	}

#pragma endregion

#pragma region VServer

	void Settings::set(VServer & VServ) {
		std::vector<VServer>::iterator it = std::find(vserver.begin(), vserver.end(), VServ);
		if (it == vserver.end()) { vserver.push_back(VServ); }
		else *it = VServ;
	}

	void Settings::del(const VServer & VServ) {
		std::vector<VServer>::iterator it = std::find(vserver.begin(), vserver.end(), VServ);
		if (it != vserver.end()) { it->clear(); vserver.erase(it); }
	}

#pragma endregion


//	GLOBAL

//	✓	access_log										/var/log/nginx/access.log;
//	✓	error_log										/var/log/nginx/error.log;
//	✓	client_max_body_size							10M;
//		error_page 404									/404.html;										error_page 404 =200 /about/index.html;
//	✓	autoindex										on;

//	SERVER	(server {)

//		listen											80;												Empty or not valid range or number
//		server_name										example.com www.example.com;
//		root											/mnt/c/www/html/example.com;					Error lectura
//		index											index.html index.htm index.php;					Empty

//		LOCATION
//
//			location 									= /404.html {	(Diferencia con el = y ~)
//			internal;
//			alias										/mnt/c/www/html/error_pages/404.html;
//			try_files									$uri $uri/ =404;								$uri $uri/ /file.html;
//			return										301 https://example.com$request_uri;
//			limit_except								GET POST {
//				deny									all;
//				return									405 /405.html;
