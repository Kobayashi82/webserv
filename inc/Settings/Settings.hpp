/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:14:05 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/27 20:19:47 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Utils.hpp"
#include "VServer.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <sstream>																						//	For std::stringstream to format strings
#include <fstream>																						//	For file streams like std::ifstream, std::ofstream
#include <cstring>																						//	For strcmp()

#include <vector>																						//	For std::vector container
#include <deque>																						//	For std::deque container
#include <map>																							//	For std::map container

#pragma region Settings

	class Settings {

		public:

			//	Variables
			static Timer								timer;											//	Class to obtain time and date related data
			static std::string							program_path;									//	Path of the executable
			static std::string							config_path;									//	Path of the default configuration file
			static std::string							server_name;									//	Name of the server
			static std::string							server_version;									//	Version of the server
			static std::string							os_name;										//	Name of the operating system

			static VServer								global;											//	Global settings
			static std::deque <VServer>					vserver;										//	VServers in a deque

			static std::map <int, std::string>			error_codes;									//	Error codes in a map
			static std::map <std::string, std::string>	mime_types;										//	MIME types in a map

			static bool									check_only;										//	Check the config file, but don't start the server
			static bool 								config_created;									//	The config file has been created
			static bool									loaded;											//	The config file loaded successfully (but may contains errors)
			static int									current_vserver;								//	Current selected VServer (-1 = None)
			static int									terminate;										//	Flags the program to exit with the value in terminate (the default value -1 don't exit)

			static const int							KEEP_ALIVE_TIMEOUT;								//	Timeout in seconds for keep-alive	(if a client is inactive for this amount of time, the connection will be closed)
			static const int							KEEP_ALIVE_REQUEST;								//	Maximum request for keep-alive 		(if a client exceeds this number of requests, the connection will be closed)

			//	Global
			static std::string data_get(std::vector<std::pair<std::string, std::string> > * data, std::string key);	//	Gets a Value from a Key in any "data" passed as a pointer
			static std::string data_get(std::vector<std::pair<std::string, std::string> > & data, std::string key);	//	Gets a Value from a Key in any "data" passed by reference
			static void	clear(bool reset = false);																	//	Deletes all Keys, their Values and optionally resets config data

			//	V-Server
			static void	set(VServer & VServ);															//	Adds or modifies a VServer
			static void	add(VServer & VServ);															//	Alias for 'set'
			static void	del(const VServer & VServ);														//	Deletes a VServer
			static void	vserver_clear();																//	Deletes all VServers

			//	Load
			static void	generate_config(const std::string & File, const std::string & path);			//	Generates a default configuration file
			static void	load();																			//	Load the default configuration file
			static void	load(const std::string & File);													//	Load a configuration file
			static void	load_args(int argc, char **argv);												//	Load a configuration file based on the defined arguments

		private:

			//	Enumerators
			enum e_section { ROOT, GLOBAL, SERVER, LOCATION, METHOD };
			
			//	Variables
			static int		line_count;																	//	Number of the current line of the configuration file (use to indicate the line of an error in the configuration file)
			static int		bracket_lvl;																//	Level of the bracket (use to parse the configuration file)

			static size_t	FILE_MAXSIZE;																//	Maximum size allowed for the configuration file

			//	Errors and MIMEs
			static void load_error_codes();																//	Load error codes into a map
			static void load_mime_types();																//	Load MIME types into a map

			//	Parser
			static void	log_servers(std::string msg, VServer * VServ);									//	Adds a log to the VServer passed as a pointer (type = MEM_ERROR)
			static void	log_access_add(std::string msg);												//	Adds an 'access' log to Global
			static void	log_error_add(std::string msg);													//	Adds an 'error' log to Global

			static void	parser(std::ifstream & infile);													//	Main parser function that reads the config file

			static void	missing_directive(VServer & VServ);
			static int	invalid_directive(std::string firstPart, int line_count, int section, VServer * VServ);
			static int	repeated_directive(const std::string & str, const std::vector<std::pair<std::string, std::string> > & data, int line_count, VServer * VServ);

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

	};

#pragma endregion
