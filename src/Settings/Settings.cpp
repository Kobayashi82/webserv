/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:27:58 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/27 22:40:49 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"

#pragma region Variables

std::map <std::string, std::string>	Settings::global;
std::vector <VServer> 				Settings::vserver;
std::string							Settings::program_path = Settings::ProgramPath();
std::string							Settings::config_path = Settings::ProgramPath() + "conf/";
int 								Settings::terminate = -1;
bool 								Settings::check_only = false;
Timer 								Settings::timer;

#pragma endregion

#pragma region Load

#pragma region Generate Config

static void generate_config(const std::string & File) {
	Settings::create_path(File); std::ofstream outfile;
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
		Settings::create_path(Settings::program_path + "www/html");
		Settings::add("error_log", "logs/error.log");
		Log::log_error("Default configuration file created succesfully");
		Settings::del("error_log");
    } else {
		Settings::add("error_log", "logs/error.log");
		Log::log_error("Could not create the default configuration file");
		Settings::del("error_log");
	}
}

#pragma endregion

#pragma region Parse Line

static int parse_line(const std::string & line) {
	std::cout << line << std::endl;
	return (0);
}

#pragma endregion

#pragma region Load File

void Settings::load(const std::string & File, bool isRegen) {
	bool isDefault = (File == Settings::config_path + "default.cfg");
	std::string	line;

    std::ifstream infile(File.c_str());
    if (infile.is_open()) {
        while (getline(infile, line)) {
			if (parse_line(line)) {
				if (isDefault) {
					if (!isRegen) {
						remove(File.c_str());
						Settings::add("error_log", "logs/error.log");
						Log::log_error("Default configuration file is corrupted, generating a default config file");
						Settings::del("error_log");
						generate_config(File);
						Settings::load(Settings::config_path + "default.cfg", true);
					} else {
						Settings::add("error_log", "logs/error.log");
						Log::log_error("Could not create the default configuration file");
						Settings::del("error_log");
					}
				} else {
					Log::log_error("Could not load the configuration file '" + File + "'");
				}
				Settings::terminate = 1;
			}
        } infile.close();
		if (isDefault)
			Log::log_access("Default configuration file loaded succesfully");
		else
			Log::log_access("Configuration file '" + File + "' loaded succesfully");
    } else {
		if (isDefault) {
			Settings::add("error_log", "logs/error.log");
			Log::log_error("Could not load the default configuration file");
			Settings::del("error_log");
		} else {
        	Log::log_error("Could not load the configuration file '" + File + "'");
		}
	}
}

#pragma endregion

#pragma region Load Default

void Settings::load() {
 	std::ifstream infile((Settings::config_path + "default.cfg").c_str());
    if (!infile.is_open()) {
		Settings::add("error_log", "logs/error.log");
		Log::log_error("Default config file missing, generating a default config file");
		Settings::del("error_log");
		generate_config(Settings::config_path + "default.cfg");
	} infile.close();
	Settings::load(Settings::config_path + "default.cfg");
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

#pragma endregion

#pragma region VServer

void Settings::set(const VServer & VServ) {
    std::vector<VServer>::iterator it = std::find(vserver.begin(), vserver.end(), VServ);
    if (it == vserver.end()) vserver.push_back(VServ);
}

void Settings::del(const VServer & VServ) {
    std::vector<VServer>::iterator it = std::find(vserver.begin(), vserver.end(), VServ);
    if (it != vserver.end()) { it->clear(); vserver.erase(it); }
}

#pragma endregion

#pragma region Clear

void Settings::clear() {
	global.clear();
	for (std::vector<VServer>::iterator it = vserver.begin(); it != vserver.end(); ++it) it->clear();
}

#pragma endregion

#pragma region Utils

void Settings::create_path(const std::string & path) {
    size_t pos = 0; std::string dir;

	if (path == "") return;
    while ((pos = path.find('/', pos)) != std::string::npos) {
        dir = path.substr(0, pos++);
        if (dir.empty()) continue;
        mkdir(dir.c_str(), 0755);
    }
}

std::string Settings::ProgramPath() {
	char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) return (std::string(result, count - 7));
	return ("/");
}

#pragma endregion
