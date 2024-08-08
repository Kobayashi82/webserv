/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:27:58 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 23:50:47 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"

#pragma region Variables

	Timer 								Settings::timer;												//	Class to obtain time and date related data
	std::string							Settings::program_path = Utils::programPath();					//	Path of the executable
	std::string							Settings::config_path = Utils::programPath() + "conf/";			//	Path of the default configuration file
	std::map <int, std::string>			Settings::error_codes;											//	Error codes in a map
	std::map <std::string, std::string>	Settings::mime_types;											//	MIME types in a map
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
	int									Settings::line_count = 0;										//	Number of the current line of the configuration file (use to indicate the line of an error in the configuration file)
	int									Settings::bracket_lvl = 0;										//	Level of the bracket (use to parse the configuration file)

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

			if (infile.is_open()) {
				while (getline(infile, line)) parse_global(infile, line);
				infile.close();
				if (bracket_lvl != 0) Log::log_error(RD "Brackets error");
				loaded_ok = true;
				if (isDefault)
					Log::log_access(G "Default configuration file loaded" NC);
				else
					Log::log_access(G "Configuration file '" Y + File + G "' loaded" NC);
			} else {
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
					Log::log_error("Default configuration file does not exist, generating a default config file");
				else if (FileStatus == 2) {
					Log::log_error("Cannot read the default configuration file, generating a default config file");
					remove(File.c_str());
				} else {
					Log::log_error("Could not load the default configuration file, generating a default config file");
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
			if (argc == 2 && !strcmp(argv[1], "-t")) {                                                                                      //  Test default settings
				check_only = true;
				std::cout << std::endl;
				load();
				if (vserver.size() == 0 && loaded_ok && Log::error.size() == 0) Log::log_error("There are no " Y "virtual servers" RD " in the configuration file");
				if (Log::error.size() == 0)			std::cout << C "\tThe configuration file is correct" NC << std::endl;
				else if (Log::error.size() == 1)	std::cout << std::endl << C "\t\tThere is "  Y << Log::error.size() << C " error in total" NC << std::endl;
				else 								std::cout << std::endl << C "\t\tThere are " Y << Log::error.size() << C " errors in total" NC << std::endl;
				std::cout << std::endl;
				terminate = 1;
			} else if (argc == 3 && (!strcmp(argv[1], "-t") || !strcmp(argv[2], "-t"))) {                                                   //  Test indicated settings
				check_only = true;
				std::cout << std::endl;
				if (!strcmp(argv[1], "-t") && !strcmp(argv[2], "-t")) {
					std::cout << RD "\t\t     Test config file" << std::endl << std::endl
							  << C "\tUsage: " Y "./webserv -t [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
					terminate = 1; return; }
				else if (strcmp(argv[1], "-t")) load(argv[1]);
				else if (strcmp(argv[2], "-t")) load(argv[2]);

				if (vserver.size() == 0 && loaded_ok && Log::error.size() == 0) Log::log_error("There are no " Y "virtual servers" RD " in the configuration file");

				if (Log::error.size() == 0)			std::cout << C "\tThe configuration file is correct" NC << std::endl;
				else if (Log::error.size() == 1)	std::cout << std::endl << C "\t\tThere is "  Y << Log::error.size() << C " error in total" NC << std::endl;
				else 								std::cout << std::endl << C "\t\tThere are " Y << Log::error.size() << C " errors in total" NC << std::endl;
				std::cout << std::endl;
				terminate = 1;
			} else if (argc > 2) {
				std::cout << std::endl << RD "\t     Incorrect number of arguments" << std::endl << std::endl
						  << C "\tUsage: " Y "./webserv [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
				terminate = 1;
			} else {
				if (argc == 1) load(); else load(argv[1]);
				if (vserver.size() == 0 && loaded_ok) Log::log_error("There are no virtual servers in the configuration file");
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
		global.clear(); vserver.clear(); config.clear(); bracket_lvl = 0; loaded_ok = false;
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
