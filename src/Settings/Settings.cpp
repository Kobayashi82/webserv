/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:27:58 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/23 19:37:45 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"
#include "Display.hpp"

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
				outfile << "http {\n"
						<< "    access_log logs/access.log;\n"
						<< "    error_log logs/error.log;\n\n"
						<< "    client_max_body_size 10M;\n\n"
						<< "    error_page 404 /404.html;\n"
						<< "    error_page 500 502 503 504 /50x.html;\n\n"
						<< "    keepalive_requests 100;\n"
						<< "    keepalive_timeout 10;\n"						
						<< "    autoindex on;\n\n"
						<< "    server {\n"
						<< "        listen 127.0.0.1:8081;\n"
						<< "        root " << path + "www/html" << ";\n"
						<< "        index index.html;\n"
						<< "        server_name default;\n\n"
						<< "        location / {\n"
						<< "            try_files $uri $uri/ =404;\n"
						<< "        }\n"
						<< "    }\n"
						<< "}" << std::endl;
				outfile.close();

				Utils::createPath(path + "www/html");
				Log::log(G "Default configuration file created succesfully" NC, Log::MEM_ACCESS);
			}
		}

	#pragma endregion

	#pragma region Load File

		void Settings::load(const std::string & File) {
			bool isDefault = (File == config_path + "default.cfg"); clear();
			std::string	line; std::ifstream infile(File.c_str());

			if (infile.is_open()) { parser(infile); infile.close(); loaded = true;
				if (BadConfig || check_only) return;
				if (isDefault)									Log::log(G "Default configuration file loaded" NC, Log::MEM_ACCESS);
				else											Log::log(G "Configuration file '" Y + File + G "' loaded" NC, Log::MEM_ACCESS);
			} else {
				BadConfig = true;
				if (isDefault)									Log::log(RD "Could not create the " Y "default configuration" RD " file" NC, Log::BOTH_ERROR);
				else {
					if (Utils::file_exists(File) == 1)			Log::log(RD "The configuration file '" Y + File + RD "' does not exist" NC, Log::BOTH_ERROR);
					else if (Utils::file_exists(File) == 2)		Log::log(RD "Cannot read the file '" Y + File + RD "'" NC, Log::BOTH_ERROR);
					else										Log::log(RD "Could not load the configuration file '" Y + File + RD "'" NC, Log::BOTH_ERROR);
				}
			}
			Log::process_logs();
		}

	#pragma endregion

	#pragma region Load Default

		void Settings::load() {
			std::string File = config_path + "default.cfg";
			int fileStatus = Utils::file_exists(File);

			if (fileStatus) {
				if (fileStatus == 1)							Log::log(RD "Default configuration file does not exist, generating a default config file" NC, Log::MEM_ACCESS);
				else if (fileStatus == 2) {						Log::log(RD "Cannot read the default configuration file, generating a default config file" NC, Log::MEM_ACCESS); remove(File.c_str()); }
				else {											Log::log(RD "Could not load the default configuration file, generating a default config file" NC, Log::MEM_ACCESS); remove(File.c_str()); }
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
				std::cout << RD "\n\t\t\tIncorrect arguments\n\n"
						  << C "\tUsage: " Y "./webserv [" B "Opional " G "-t" C " or " G "-i" Y"] [" B "Opional " G "settings file" Y "]\n\n" NC
						  << Y "   -t:  " C "Checks the configuration file for syntax and errors\n" NC
						  << Y "   -i:  " C "Run the program in console mode, without a graphical interface\n" NC << std::endl;

			//	Option -i: Run the program in console mode
			} else if ((argc == 2 || argc == 3) && !strcmp(argv[1], "-i")) {
				Display::RawModeDisabled = true; Display::ForceRawModeDisabled = true; Display::Logo();

			//	Option -t: Checks the configuration file
			} else if ((argc == 2 || argc == 3) && !strcmp(argv[1], "-t")) {		check_only = true;
				std::cout << std::endl;
				
				if (argc == 2) load();
				if (argc == 3) load(argv[2]);
				
				if (BadConfig == false && global.log.error.size() == 0 && global.log.access.size() > 0) std::cout << std::endl;

				if (BadConfig == false && global.log.error.size() > 0)				std::cout << C "\n\t\tThe configuration file is correct with some " Y "minor errors" NC << std::endl;
				else if (BadConfig == false)										std::cout << C "\t\tThe configuration file is correct" NC << std::endl;
				else if (global.log.error.size() == 1)								std::cout << C "\n\t\tThere is "  Y "1" C " error in total" NC << std::endl;
				else 																std::cout << C "\n\t\tThere are " Y << global.log.error.size() << C " errors in total" NC << std::endl;

				terminate = 1;
			//	Incorrect number of arguments
			} else if (argc > 2) {													terminate = 1;
				std::cout << RD "\n\t\t     Incorrect number of arguments\n\n"
						  << C "\tUsage: " Y "./webserv [" B "Opional " G "-t" C " or " G "-i" Y"] [" B "Opional " G "settings file" Y "]\n\n" NC
						  << Y "   -t:  " C "Checks the configuration file for syntax and errors\n" NC
						  << Y "   -i:  " C "Run the program in console mode, without a graphical interface\n" NC << std::endl;
			}

			//	Load the configuration file
			if (terminate == -1) {
				Display::enableRawMode();
				if (Display::RawModeDisabled || Display::ForceRawModeDisabled) std::cout << std::endl;

				if (argc == 1 || (argc == 2 && !strcmp(argv[1], "-i"))) load();
				else if (argc == 2 && strcmp(argv[1], "-i")) load(argv[1]);
				else if (argc == 3) load(argv[2]);

				if (BadConfig == false) {
					Settings::global.status = true;
				} else {
					Log::log(RD "Could not load configuration file" NC, Log::BOTH_ERROR);
					if (Display::RawModeDisabled || Display::ForceRawModeDisabled) terminate = 1;
				}
			}
		}

#pragma endregion

#pragma endregion
