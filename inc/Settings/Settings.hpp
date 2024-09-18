/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:14:05 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 15:35:01 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Utils.hpp"
#include "VServer.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <sstream>																						//	For std::stringstream to format strings
#include <fstream>																						//	For file streams like std::ifstream, std::ofstream

#include <vector>																						//	For std::vector container
#include <deque>																						//	For std::deque container
#include <map>																							//	For std::map container

#include <cstring>																						//	For strcmp()

#pragma region Settings

class Settings {

	public:

		//	Variables
		static Timer								timer;												//	Class to obtain time and date related data
		static std::string							program_path;										//	Path of the executable
		static std::string							config_path;										//	Path of the default configuration file

		static VServer								global;												//	Global settings in a vector
		static std::deque <VServer>					vserver;											//	V-Servers in a deque

		static std::map <int, std::string>			error_codes;										//	Error codes in a map
		static std::map <std::string, std::string>	mime_types;											//	MIME types in a map

		static bool									check_only;											//	Check the config file, but don't start the server
		static bool 								config_created;										//	The config file has been created
		static bool									loaded;												//	The config file loaded successfully (but may contains errors)
		static int									current_vserver;									//	Current selected V-Server (-1 = None)
		static int									terminate;											//	Flag the program to exit with the value in terminate (the default value of -1 don't exit)

		static const int							KEEP_ALIVE_TIMEOUT;									//	Timeout in seconds for keep-alive (if a client is inactive for this amount of time, the connection will be closed)
		static const int							KEEP_ALIVE_REQUEST;									//	Maximum request for keep-alive (if a client exceeds this number of requests, the connection will be closed)

		//	Global
		static std::string data_get(std::vector<std::pair<std::string, std::string> > * data, std::string key);		//	Get a value from a key in any "data" passed as a pointer
		static std::string data_get(std::vector<std::pair<std::string, std::string> > & data, std::string key);		//	Get a value from a key in any "data" passed as a reference
		static void	clear(bool reset = false);																		//	Delete all Keys, his Values and optionally reset config data

		//	V-Server
		static void	set(VServer & VServ);																//	Add or modify a V-Server
		static void	add(VServer & VServ);																//	Alias for 'set'
		static void	del(const VServer & VServ);															//	Delete a V-Server
		static void	vserver_clear();																	//	Delete all V-Servers

		//	Load
		static void	generate_config(const std::string & File, const std::string & path);
		static void	load();																				//	Load the default configuration file
		static void	load(const std::string & File);														//	Load a configuration file
		static void	load_args(int argc, char **argv);													//	Load a configuration file based on the defined arguments

	private:

		//	Variables
		enum e_section { ROOT, GLOBAL, SERVER, LOCATION, METHOD };										//	Enumarator for the sections of the config file
		
		static int		line_count;																		//	Number of the current line of the configuration file (use to indicate the line of an error in the configuration file)
		static int		bracket_lvl;																	//	Level of the bracket (use to parse the configuration file)
		static bool		BadConfig;																		//	Indicate if there are errors in the config file
		static size_t	FILE_MAXSIZE;																	//	Maximum size allowed for the configuration file

		//	Errors and MIMEs
		static void load_error_codes();																	//	Load error codes in a map
		static void load_mime_types();																	//	Load MIME types in a map

		//	Parser
		static void	log_servers(std::string msg, VServer * VServ);
		static void	log_access_add(std::string msg);
		static void	log_error_add(std::string msg);

		static int	parse_path(const std::string & firstPart, std::string & str, bool isFile, bool check_path, bool check_write, VServer * VServ);
		static int	parse_keepalive_timeout(std::string & str, VServer * VServ);
		static int	parse_keepalive_requests(std::string & str, VServer * VServ);
		static int	parse_log_rotatesize(std::string & str, VServer * VServ);
		static int	parse_log_rotate(std::string & str, VServer * VServ);
		static int	parse_body_size(std::string & str, VServer * VServ);
		static int	parse_errors(const std::string & firstPart, const std::string & secondPart, VServer * VServ);
		static int	parse_errors(const std::string & firstPart, const std::string & secondPart, VServer & VServ);
		static int	parse_errors(const std::string & firstPart, const std::string & secondPart, Location & Loc);
		static int	parse_autoindex(std::string & str, VServer * VServ);
		static int	parse_index(std::string & str, VServer * VServ);
		static int	parse_listen(std::string & str, VServer & VServ);
		static int	parse_return(std::string & str, VServer * VServ);
		static int	parse_alias(std::string & firstPart, std::string & str, VServer * VServ);
		static int	parse_try_files(std::string & str, VServer * VServ);
		static int	parse_allow(std::string & str, VServer * VServ);
		static int	parse_deny(std::string & str, VServer * VServ);
		static int	parse_method(std::string & str, VServer * VServ);
		static int	parse_location(std::string & str, VServer * VServ);
		static int	parse_cgi(const std::string & firstPart, const std::string & secondPart, VServer * VServ);
		static int	parse_cgi(const std::string & firstPart, const std::string & secondPart, VServer & VServ);
		static int	parse_cgi(const std::string & firstPart, const std::string & secondPart, Location & Loc);

		static int	repeated_directive(const std::string & str, const std::vector<std::pair<std::string, std::string> > & data, int line_count, VServer * VServ);
		static int	invalid_directive(std::string firstPart, int line_count, int section, VServer * VServ);
		static void	missing_directive(VServer & VServ);

		static void	parser(std::ifstream & infile);														//	Main parser function that read the config file

};

#pragma endregion
