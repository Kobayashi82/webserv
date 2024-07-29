/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:27:58 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/29 14:58:20 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"

#pragma region Variables

std::map <std::string, std::string>	Settings::global;
std::vector <VServer> 				Settings::vserver;
std::string							Settings::program_path = Settings::ProgramPath();
std::string							Settings::config_path = Settings::ProgramPath() + "conf/";
int 								Settings::terminate = -1;
int									Settings::bracket_lvl = 0;
bool 								Settings::check_only = false;
bool 								Settings::loaded_ok = false;
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
		Log::log_error("Default configuration file created succesfully", NULL, true);
    } else {
		Log::log_error("Could not create the default configuration file", NULL, true);
	}
}

#pragma endregion

#pragma region Parse Line

static void trim(std::string &str) {
    // Eliminar espacios en blanco al principio
    std::string::iterator start = str.begin();
    std::string::iterator end = str.end();

    while (start != str.end() && std::isspace(static_cast<unsigned char>(*start))) start++;

    // Encontrar el carácter '#' y ajustar el final de la cadena
    std::string::iterator hashPos = std::find(start, str.end(), '#');
    if (hashPos != str.end()) end = hashPos;

    // Eliminar espacios en blanco al final
    while (end != start && std::isspace(static_cast<unsigned char>(*(end - 1)))) --end;
    str = std::string(start, end);
}

static void toLower(std::string & str) {
    for (size_t i = 0; i < str.size(); ++i) str[i] = std::tolower(static_cast<unsigned char>(str[i]));
}

static int brackets(std::string & str) {
	if (!str.empty() && str[str.size() - 1] == '{') {
		str.erase(str.size() - 1);
		Settings::bracket_lvl++;
		return (1);
	}
	if (!str.empty() && str[str.size() - 1] == '}') {
		str.erase(str.size() - 1);
		Settings::bracket_lvl--;
		return (-1);
	}
	return (0);
}

static void parse_body_size(std::string & str) {
	if (str.empty() || std::isdigit(str[str.size() - 1])) return ;

    long multiplier = 1;
    char unit = str[str.size() - 1];
    std::string numberPart = str.substr(0, str.size() - 1);

    switch (unit) {
        case 'K': multiplier = 1024; break;
        case 'M': multiplier = 1024 * 1024; break;
        case 'G': multiplier = 1024 * 1024 * 1024; break;
    }

    std::stringstream ss(numberPart);
    long number; ss >> number; if (ss.fail()) return;
    
    std::stringstream resultStream;
    resultStream << number * multiplier;
    str = resultStream.str();
}

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

static int parser_location(std::ifstream & infile, std::string & line, VServer & VServ) {
	bool inLimit = false; Location Loc;
	do {
		std::string firstPart, secondPart;
		trim(line); if (line.empty()) continue;
		std::istringstream stream(line);

		stream >> firstPart; trim(firstPart); toLower(firstPart);

		std::getline(stream, secondPart); trim(secondPart);

		if (!firstPart.empty() && firstPart[firstPart.size() - 1] == ';') firstPart.erase(firstPart.size() - 1);
		if (!secondPart.empty() && secondPart[secondPart.size() - 1] != '{' && secondPart[secondPart.size() - 1] != '}' && secondPart[secondPart.size() - 1] != ';' && firstPart != "{" && firstPart != "}") return (1);
		if (!secondPart.empty() && secondPart[secondPart.size() - 1] == ';') secondPart.erase(secondPart.size() - 1);

		if (brackets(firstPart) == -1 || brackets(secondPart) == -1) {
			if (inLimit) inLimit = false; else { VServ.add(Loc); break; }
		}

		if (firstPart.empty()) continue;
		if (firstPart == "limit_except") inLimit = true;
		if (firstPart == "client_max_body_size") parse_body_size(secondPart);
		if (firstPart == "error_page") parse_errors(firstPart, secondPart, Loc); else Loc.add(firstPart, secondPart);
		
	} while (getline(infile, line));
	return (0);
}

static int parser_server(std::ifstream & infile, std::string & line) {
	VServer VServ;
	do {
		std::string firstPart, secondPart;
		trim(line); if (line.empty()) continue;
		std::istringstream stream(line);
		
		stream >> firstPart; trim(firstPart); toLower(firstPart);

		if (firstPart == "location") { if (parser_location(infile, line, VServ)) return (1); else continue; }

		std::getline(stream, secondPart); trim(secondPart);
		
		if (!firstPart.empty() && firstPart[firstPart.size() - 1] == ';') firstPart.erase(firstPart.size() - 1);
		if (!secondPart.empty() && secondPart[secondPart.size() - 1] != '{' && secondPart[secondPart.size() - 1] != '}' && secondPart[secondPart.size() - 1] != ';' && firstPart != "{" && firstPart != "}" && firstPart != "location") return (1);
		if (!secondPart.empty() && secondPart[secondPart.size() - 1] == ';') secondPart.erase(secondPart.size() - 1);
		
		if (brackets(firstPart) == -1 || brackets(secondPart) == -1) { Settings::add(VServ); break; }

		if (firstPart.empty()) continue;
		if (firstPart == "client_max_body_size") parse_body_size(secondPart);
		if (firstPart == "error_page") parse_errors(firstPart, secondPart, VServ);
		else if (firstPart != "server") VServ.add(firstPart, secondPart);

	} while (getline(infile, line));
	return (0);
}

static int parse_line(std::ifstream & infile, std::string & line) {
	std::string firstPart, secondPart;
	trim(line); if (line.empty()) return (0);
	std::istringstream stream(line);

	stream >> firstPart; trim(firstPart); toLower(firstPart);

	if (firstPart == "server") { return (parser_server(infile, line)); }

	std::getline(stream, secondPart); trim(secondPart);

	if (!firstPart.empty() && firstPart[firstPart.size() - 1] == ';') firstPart.erase(firstPart.size() - 1);
	if (!secondPart.empty() && secondPart[secondPart.size() - 1] != '{' && secondPart[secondPart.size() - 1] != '}' && secondPart[secondPart.size() - 1] != ';' && firstPart != "{" && firstPart != "}" && firstPart != "server" && firstPart != "location") return (1);
	if (!secondPart.empty() && secondPart[secondPart.size() - 1] == ';') secondPart.erase(secondPart.size() - 1);

	brackets(firstPart); brackets(secondPart);

	if (firstPart.empty()) return (0);
	if (firstPart == "client_max_body_size") parse_body_size(secondPart);
	if (firstPart == "error_page") parse_errors(firstPart, secondPart); else Settings::add(firstPart, secondPart);

	return (0);
}

#pragma endregion

#pragma region Load File

static int file_exists(const std::string & File) {
	if (access(File.c_str(), F_OK) < 0) return (1);
	if (access(File.c_str(), R_OK) < 0) return (2);
    return (0);
}

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
	global.clear(); Settings::bracket_lvl = 0;
	for (std::vector<VServer>::iterator it = vserver.begin(); it != vserver.end(); ++it) it->clear();
	vserver.clear();
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
