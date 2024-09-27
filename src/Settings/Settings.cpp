/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:27:58 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/27 20:23:02 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"
#include "Display.hpp"

#pragma region Variables

	Timer 								Settings::timer;												//	Class to obtain time and date related data
	std::string							Settings::program_path = Utils::programPath();					//	Path of the executable
	std::string							Settings::config_path = Utils::programPath();					//	Path of the default configuration file
	std::string							Settings::server_name = "Webserv";								//	Name of the server
	std::string							Settings::server_version = "1.0";								//	Version of the server
	std::string							Settings::os_name = Utils::get_OSname();						//	Name of the operating system

	VServer								Settings::global;												//	Global settings
	std::deque <VServer> 				Settings::vserver;												//	VServers in a deque
	
	std::map <int, std::string>			Settings::error_codes;											//	Error codes in a map
	std::map <std::string, std::string>	Settings::mime_types;											//	MIME types in a map

	bool 								Settings::check_only = false;									//	Check the config file, but don't start the server
	bool 								Settings::config_created = false;								//	The config file has been created
	bool 								Settings::loaded = false;										//	The config file loaded successfully (but may contains errors)
	int									Settings::current_vserver = -1;									//	Current selected VServer (-1 = None)
	int 								Settings::terminate = -1;										//	Flags the program to exit with the value in terminate (the default value -1 don't exit)

	const int							Settings::KEEP_ALIVE_TIMEOUT = 75;								//	Timeout in seconds for keep-alive (if a client is inactive for this amount of time, the connection will be closed)
	const int							Settings::KEEP_ALIVE_REQUEST = 500;								//	Maximum request for keep-alive (if a client exceeds this number of requests, the connection will be closed)

	int									Settings::line_count = 0;										//	Number of the current line of the configuration file (use to indicate the line of an error in the configuration file)
	int									Settings::bracket_lvl = 0;										//	Level of the bracket (use to parse the configuration file)

	size_t								Settings::FILE_MAXSIZE = 1 * 1024 * 1024;						//	Maximum size allowed for the configuration file

#pragma endregion

#pragma region Global

	#pragma region Data Get

		std::string Settings::data_get(std::vector<std::pair<std::string, std::string> > * data, std::string key) {
			for (std::vector<std::pair<std::string, std::string> >::const_iterator it = data->begin(); it != data->end(); ++it)
				if (it->first == key) return it->second;

			return ("");
		}

		std::string Settings::data_get(std::vector<std::pair<std::string, std::string> > & data, std::string key) {
			for (std::vector<std::pair<std::string, std::string> >::const_iterator it = data.begin(); it != data.end(); ++it)
				if (it->first == key) return it->second;

			return ("");
		}

	#pragma endregion

    #pragma region Clear

		void Settings::clear(bool reset) {
			vserver_clear(); global.data.clear(); global.method.clear();
			if (reset) {
				terminate = -1;
				loaded = false;
				bracket_lvl = 0;
				config_created = false;
				line_count = 0;
				current_vserver = -1;
				global.bad_config = false;
				global.status = false;
				global.addresses.clear();
				global.config_displayed = false;
				global.config.clear();
				global.config_index = 0;
				global.log.clear();
				global.log_index = 0;
				global.autolog = true;
			}
		}

	#pragma endregion

#pragma endregion

#pragma region VServer

	#pragma region Set/Add

		void Settings::set(VServer & VServ) {
			std::deque <VServer>::iterator it = std::find(vserver.begin(), vserver.end(), VServ);
			if (it == vserver.end()) vserver.push_back(VServ);
			else *it = VServ;
		}

		void Settings::add(VServer & VServ) { set(VServ); }

	#pragma endregion

	#pragma region Del

		void Settings::del(const VServer & VServ) {
			std::deque <VServer>::iterator it = std::find(vserver.begin(), vserver.end(), VServ);
			if (it != vserver.end()) { it->clear(); vserver.erase(it); }
		}

	#pragma endregion

	#pragma region Clear

		void Settings::vserver_clear() {
			for (std::deque <VServer>::iterator it = vserver.begin(); it != vserver.end(); ++it) it->clear();
			vserver.clear();
		}

	#pragma endregion

#pragma endregion

#pragma region Load

	#pragma region Generate Config

		void Settings::generate_config(const std::string & File, const std::string & path) {
			Utils::createPath(File); std::ofstream outfile;
			outfile.open(File.c_str(), std::ios_base::app);

			if (outfile.is_open()) {
				outfile << "http {\n"
						<< "\taccess_log logs/access.log;\n"
						<< "\terror_log logs/error.log;\n\n"

						<< "\tbody_maxsize 10M;\n\n"

						<< "\terror_page 404 /404.html;\n"
						<< "\terror_page 500 502 503 504 /50x.html;\n\n"

						<< "\tkeepalive_requests 100;\n"
						<< "\tkeepalive_timeout 10;\n"
						<< "\tautoindex on;\n\n"

						<< "\tcgi .py /usr/bin/python3;\n"
						<< "\tcgi .php /usr/bin/php;\n\n"

						<< "\tserver {\n"
						<< "\t\tlisten 8081;\n"
						<< "\t\troot " << path + "www/html/" << ";\n"
						<< "\t\tindex index.html;\n"
						<< "\t\tserver_name default;\n\n"

						<< "\t\tlocation / {\n"
						<< "\t\t\ttry_files $uri $uri/ =404;\n"
						<< "\t\t}\n"
						<< "\t}\n"
						<< "}" << std::endl;
				outfile.close();

				config_created = true;
				Utils::createPath(path + "www/html/");
				Utils::createPath(path + "logs/");
				log_access_add("---"); log_access_add(G "Default configuration file created succesfully" NC); log_access_add("---");
			}
		}

	#pragma endregion

	#pragma region Load File

		void Settings::load(const std::string & file) {
			bool isDefault = (file == config_path + "default.cfg"); clear();

			size_t filesize = Utils::filesize(file);
			if (filesize != std::string::npos && filesize > FILE_MAXSIZE) {
				if (isDefault)									log_access_add(RD "The " Y "default configuration" RD " file is larger than " Y "1 MB" NC);
				else											log_access_add(RD "The configuration file '" + Utils::ltos(FILE_MAXSIZE) + Y + file + RD "' is larger than " Y "1 MB" NC);
				log_access_add("---");
				global.bad_config = true;
				return;
			}

			std::string	line; std::ifstream infile(file.c_str());

			if (infile.is_open()) { parser(infile); infile.close(); loaded = true;
				if (check_only) return;
				if (global.bad_config) { log_access_add("---"); return; }

				bool print_sep = (global.log.error.size() > 0 && !(global.log.error.size() == 1 && config_created));
				for (std::deque<VServer>::iterator it = vserver.begin(); it != vserver.end(); ++it)
					if (it->log.error.size() > 0) print_sep = true;
				if (print_sep)								{																									log_access_add("---"); }
				if (isDefault)								{	log_access_add(G "Default configuration file loaded" NC);										log_access_add("---"); }
				else										{	log_access_add(G "Configuration file '" Y + file + G "' loaded" NC);							log_access_add("---"); }
			} else {
				global.bad_config = true;
				if (isDefault)								{	log_error_add(RD "Could not create the " Y "default configuration" RD " file" NC);				log_access_add("---"); }
				else {
					if (Utils::file_exists(file) == 1)		{	log_error_add(RD "The configuration file '" Y + file + RD "' does not exist" NC);				log_access_add("---"); }
					else if (Utils::file_exists(file) == 2)	{	log_error_add(RD "Cannot read the file '" Y + file + RD "'" NC);								log_access_add("---"); }
					else									{	log_error_add(RD "Could not load the configuration file '" Y + file + RD "'" NC);				log_access_add("---"); }
				}
			}
			//Log::process_logs();
		}

	#pragma endregion

	#pragma region Load Default

		void Settings::load() {
			std::string File = config_path + "default.cfg";
			int fileStatus = Utils::file_exists(File);

			if (fileStatus) {
				if (fileStatus == 1)							log_error_add(RD "Default configuration file does not exist, generating a default config file" NC);
				else if (fileStatus == 2) {						log_error_add(RD "Cannot read the default configuration file, generating a default config file" NC); 	remove(File.c_str()); }
				else {											log_error_add(RD "Could not load the default configuration file, generating a default config file" NC); remove(File.c_str()); }
				generate_config(File, program_path);
			}

			load(File);
		}

	#pragma endregion

	#pragma region Load Args

		void Settings::load_args(int argc, char **argv) {
			load_error_codes(); load_mime_types();
			//	Incorrect arguments
			if (argc == 3 && (!strcmp(argv[2], "-i") || !strcmp(argv[2], "-t"))) {	terminate = 1; check_only = true;
				if (tcgetpgrp(STDIN_FILENO) != getpgrp()) return;
				std::cout << RD "\n\t\t\tIncorrect arguments\n\n"
						  << C "\tUsage: " Y "./" + Utils::strToLower(Settings::server_name) + " [" B "Opional " G "-t" C " or " G "-i" Y"] [" B "Opional " G "settings file" Y "]\n\n" NC
						  << Y "   -t:  " C "Checks the configuration file for syntax and errors\n" NC
						  << Y "   -i:  " C "Run the program in console mode, without a graphical interface\n" NC << std::endl;

			//	Option -i: Run the program in console mode
			} else if ((argc == 2 || argc == 3) && !strcmp(argv[1], "-i")) {
				Display::RawModeDisabled = true; Display::ForceRawModeDisabled = true;
				if (tcgetpgrp(STDIN_FILENO) == getpgrp()) Display::Logo();

			//	Option -t: Checks the configuration file
			} else if ((argc == 2 || argc == 3) && !strcmp(argv[1], "-t")) {		check_only = true;
				if (tcgetpgrp(STDIN_FILENO) != getpgrp()) { terminate = 1; return; }
				std::cout << std::endl;
				
				if (argc == 2) load();
				if (argc == 3) load(argv[2]);
				
				int errors = global.log.error.size();
				for (std::deque<std::string>::const_iterator it = global.log.both.begin(); it != global.log.both.end(); ++it) {
					if (it->empty()) continue;
					if (global.log.both.size() > 1 && Utils::str_nocolor(*it).size() > 23 + 28 && Utils::str_nocolor(*it).substr(23, 28) == "There are no virtual servers") {
						std::cout << std::endl;
						if (Utils::str_nocolor(*it).substr(23, 38) == "There are no virtual servers available") errors--;
					}
					if (*it != "---" && it->size() >= 98) { std::cout << " " << it->substr(98) << NC << std::endl; }
				}

				bool disabled_servers = false;
				for (std::deque<VServer>::const_iterator it = vserver.begin(); it != vserver.end(); ++it)
					if (it->bad_config == true) { disabled_servers = true; break; }

				if (global.bad_config == false && !disabled_servers && errors)		std::cout << C "\n\t\tThe configuration file is correct with some " Y "minor errors" NC << std::endl;
				else if (global.bad_config == false && errors)						std::cout << C "\n\t\tThe configuration file is correct with some " Y "virtual servers" C " disabled" NC << std::endl;
				else if (global.bad_config == false)								std::cout << C "\t\tThe configuration file is correct" NC << std::endl;
				else if (errors == 1)												std::cout << C "\n\t\tThere is "  Y "1" C " error in total" NC << std::endl;
				else 																std::cout << C "\n\t\tThere are " Y << errors << C " errors in total" NC << std::endl;

				terminate = 1;
			//	Incorrect number of arguments
			} else if (argc > 2) {													terminate = 1;
				if (tcgetpgrp(STDIN_FILENO) != getpgrp()) return;
				std::cout << RD "\n\t\t     Incorrect number of arguments\n\n"
						  << C "\tUsage: " Y "./" + Utils::strToLower(Settings::server_name) + " [" B "Opional " G "-t" C " or " G "-i" Y"] [" B "Opional " G "settings file" Y "]\n\n" NC
						  << Y "   -t:  " C "Checks the configuration file for syntax and errors\n" NC
						  << Y "   -i:  " C "Run the program in console mode, without a graphical interface\n" NC << std::endl;
			}

			//	Load the configuration file
			if (terminate == -1) {
				if (tcgetpgrp(STDIN_FILENO) != getpgrp()) { Display::RawModeDisabled = true; Display::ForceRawModeDisabled = true; Display::background = true; }

				Display::enableRawMode();
				if (!Display::background && (Display::RawModeDisabled || Display::ForceRawModeDisabled)) { std::cout << std::endl; }

				if (argc == 1 || (argc == 2 && !strcmp(argv[1], "-i"))) load();
				else if (argc == 2 && strcmp(argv[1], "-i")) load(argv[1]);
				else if (argc == 3) load(argv[2]);

				bool no_servers = true;
				for (std::deque<VServer>::iterator it = vserver.begin(); it != vserver.end(); ++it) {
					if (it->log.error.size() > 0) { it->log.access_add("---"); it->log.both_add("---"); }
					if (it->bad_config == false) no_servers = false;
				}
				if (global.bad_config == false && no_servers == false) {
					Settings::global.status = true;
					if (!Display::background && (Display::RawModeDisabled || Display::ForceRawModeDisabled)) {
						std::cout << " " << Y "-----------------------------------------------------------------------------------------" << NC << std::endl;
						for (std::deque<std::string>::const_iterator it = global.log.both.begin(); it != global.log.both.end(); ++it) {
							if (*it == "---")			std::cout << " " << Y "-----------------------------------------------------------------------------------------" << NC << std::endl;
							else if (it->size() >= 98)	std::cout << " " << it->substr(98) << NC << std::endl;
						}
					}
				} else {
					if (!Display::background && (Display::RawModeDisabled || Display::ForceRawModeDisabled)) {
						std::cout << " " << Y "-----------------------------------------------------------------------------------------" << NC << std::endl;
						for (std::deque<std::string>::const_iterator it = global.log.both.begin(); it != global.log.both.end(); ++it) {
							if (*it == "---")			std::cout << " " << Y "-----------------------------------------------------------------------------------------" << NC << std::endl;
							else if (it->size() >= 98)	std::cout << " " << it->substr(98) << NC << std::endl;
						}
					}
					if (Display::RawModeDisabled || Display::ForceRawModeDisabled)	terminate = 1;
				}
			}
		}

#pragma endregion

#pragma endregion
