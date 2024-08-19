/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:27:58 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/19 20:28:51 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"

#pragma region Variables

	Timer 								Settings::timer;												//	Class to obtain time and date related data
	std::string							Settings::program_path = Utils::programPath();					//	Path of the executable
	std::string							Settings::config_path = Utils::programPath();					//	Path of the default configuration file
	
	VServer								Settings::global;												//	Global settings in a vector
	std::deque <VServer> 				Settings::vserver;												//	V-Servers in a deque
	
	std::map <int, std::string>			Settings::error_codes;											//	Error codes in a map
	std::map <std::string, std::string>	Settings::mime_types;											//	MIME types in a map

	bool 								Settings::check_only = false;									//	Check the config file, but don't start the server
	bool 								Settings::loaded = false;										//	The config file loaded successfully (but may contains errors)
	int									Settings::current_vserver = -1;									//	Current selected V-Server (-1 = None)
	int 								Settings::terminate = -1;										//	Flag the program to exit with the value in terminate (the default value of -1 don't exit)

	bool 								Settings::BadConfig = false;									//	Indicate if there are errors in the config file
	int									Settings::line_count = 0;										//	Number of the current line of the configuration file (use to indicate the line of an error in the configuration file)
	int									Settings::bracket_lvl = 0;										//	Level of the bracket (use to parse the configuration file)

#pragma endregion

#pragma region Global

    #pragma region Clear

		void Settings::clear(bool reset) { vserver_clear(); global.data.clear(); if (reset) { bracket_lvl = 0; loaded = false; }}

	#pragma endregion

#pragma endregion

#pragma region VServer

	#pragma region Set/Add

		void Settings::set(VServer & VServ) {
			std::deque <VServer>::iterator it = std::find(vserver.begin(), vserver.end(), VServ);
			if (it == vserver.end()) { vserver.push_back(VServ); }
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

		static void generate_config(const std::string & File, const std::string & path) {
			Utils::createPath(File); std::ofstream outfile;
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
						<< "    root " << path + "www/html" << ";" << std::endl
						<< "    index index.html;" << std::endl
						<< "    server_name default;" << std::endl << std::endl
						<< "    location / {" << std::endl
						<< "        try_files $uri $uri/ =404;" << std::endl
						<< "    }" << std::endl
						<< "}" << std::endl;
				outfile.close();
				Utils::createPath(path + "www/html");
				Log::log_error("Default configuration file created succesfully");
			}
		}

	#pragma endregion

	#pragma region Load File

		void Settings::load(const std::string & File) {
			bool isDefault = (File == config_path + "default.cfg"); clear();
			std::string	line; std::ifstream infile(File.c_str());

			if (infile.is_open()) { parser(infile); infile.close();
				if (bracket_lvl != 0) Log::log_error(RD "Brackets error");
				loaded = true;
				Log::check_logs();
				if (BadConfig) { vserver_clear(); return; }
				if (isDefault)
					Log::log_access(G "Default configuration file loaded" NC);
				else
					Log::log_access(G "Configuration file '" Y + File + G "' loaded" NC);
			} else {
				BadConfig = true;
				if (isDefault) {
					Log::log_error(RD "Could not create the " Y "default configuration" RD " file" NC);
				} else {
					if (Utils::file_exists(File) == 1)
						Log::log_error(RD "The configuration file '" Y + File + RD "' does not exist" NC);
					else if (Utils::file_exists(File) == 2)
						Log::log_error(RD "Cannot read the file '" Y + File + RD "'" NC);
					else
						Log::log_error(RD "Could not load the configuration file '" Y + File + RD "'" NC);
				}
			}
		}

	#pragma endregion

	#pragma region Load Default

		void Settings::load() {
			std::string File = config_path + "default.cfg";
			int FileStatus = Utils::file_exists(File);

			if (FileStatus) {
				if (FileStatus == 1)
					Log::log_error(RD "Default configuration file does not exist, generating a default config file" NC);
				else if (FileStatus == 2) {
					Log::log_error(RD "Cannot read the default configuration file, generating a default config file" NC);
					remove(File.c_str());
				} else {
					Log::log_error(RD "Could not load the default configuration file, generating a default config file" NC);
					remove(File.c_str());
				}
				generate_config(File, program_path);
			}
			load(File);
		}

	#pragma endregion

	#pragma region Load Args

		void Settings::load_args(int argc, char **argv) {
			load_error_codes(); load_mime_types();
			if ((argc == 2 && !strcmp(argv[1], "-i")) || (argc == 3 && (!strcmp(argv[2], "-i")))) {
				if (argc == 3 && !strcmp(argv[1], "-i") && !strcmp(argv[2], "-i")) {
					std::cout << std::endl; check_only = true;
					std::cout << RD "\t\t     Test config file" << std::endl << std::endl
							  << C "\tUsage: " Y "./webserv -t [" B "Opional " G "settings file" Y "]" NC << std::endl;
					terminate = 1; return; }
				Display::RawModeDisabled = true; Display::ForceRawModeDisabled = true; argc--; Display::Logo(); }
			if (argc == 2 && !strcmp(argv[1], "-t")) {                                                                                      //  Test default settings
				check_only = true;
				std::cout << std::endl;
				load();
				if (vserver.size() == 0 && loaded && global.log.error.size() == 0) Log::log_error("There are no " Y "virtual servers" RD " in the configuration file");
				if (global.log.error.size() == 0)			std::cout << C "\tThe configuration file is correct" NC << std::endl;
				else if (global.log.error.size() == 1)	std::cout << std::endl << C "\t\tThere is "  Y << global.log.error.size() << C " error in total" NC << std::endl;
				else 								std::cout << std::endl << C "\t\tThere are " Y << global.log.error.size() << C " errors in total" NC << std::endl;
				terminate = 1;
			} else if (argc == 3 && (!strcmp(argv[1], "-t") || !strcmp(argv[2], "-t"))) {                                                   //  Test indicated settings
				check_only = true;
				std::cout << std::endl;
				if (!strcmp(argv[1], "-t") && !strcmp(argv[2], "-t")) {
					std::cout << RD "\t\t     Test config file" << std::endl << std::endl
							  << C "\tUsage: " Y "./webserv -t [" B "Opional " G "settings file" Y "]" NC << std::endl;
					terminate = 1; return; }
				else if (strcmp(argv[1], "-t")) load(argv[1]);
				else if (strcmp(argv[2], "-t")) load(argv[2]);

				if (vserver.size() == 0 && loaded && global.log.error.size() == 0) Log::log_error("There are no " Y "virtual servers" RD " in the configuration file");

				if (global.log.error.size() == 0)			std::cout << C "\tThe configuration file is correct" NC << std::endl;
				else if (global.log.error.size() == 1)	std::cout << std::endl << C "\t\tThere is "  Y << global.log.error.size() << C " error in total" NC << std::endl;
				else 								std::cout << std::endl << C "\t\tThere are " Y << global.log.error.size() << C " errors in total" NC << std::endl;
				terminate = 1;
			} else if (argc > 2) {
				std::cout << std::endl << RD "\t     Incorrect number of arguments" << std::endl << std::endl
						  << C "\tUsage: " Y "./webserv [" B "Opional " G "settings file" Y "]" NC << std::endl;
				terminate = 1;
			} else {
				Display::enableRawMode();
				if (Display::RawModeDisabled || Display::ForceRawModeDisabled) std::cout << std::endl;
				if (argc == 1) load(); else load(argv[1]);
				if (!BadConfig && vserver.size() == 0 && loaded)	Log::log_error(RD "There are no virtual servers in the configuration file");
				else if (vserver.size() == 0) {
					if (Display::RawModeDisabled || Display::ForceRawModeDisabled) std::cout << std::endl;
					Log::log_error(RD "Could not load configuration file");
				} else Settings::global.status = true;
				if ((Display::RawModeDisabled || Display::ForceRawModeDisabled) && global.log.error.size() > 0) terminate = 1;
			}
		}

#pragma endregion

#pragma endregion
