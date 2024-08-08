/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:14:05 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 23:44:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Colors.hpp"
#include "Utils.hpp"
#include "Timer.hpp"
#include "VServer.hpp"
#include "Log.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <sstream>																						//	For std::stringstream to format strings
#include <fstream>																						//	For file streams like std::ifstream, std::ofstream

#include <vector>																						//	For std::vector container
#include <deque>																						//	For std::deque container
#include <map>																							//	For std::map container

#include <cstring>																						//	For strcmp()

class Settings {

	private:

		//	Variables
		static bool									loaded_ok;											//	The config file loaded successfully (but may contains errors)
		static bool									BadConfig;											//	Indicate if there are errors in the config file
		static int									line_count;											//	Number of the current line of the configuration file (use to indicate the line of an error in the configuration file)
		static int									bracket_lvl;										//	Level of the bracket (use to parse the configuration file)

		//	Errors and MIMEs
		static void load_error_codes();																	//	Load error codes in a map
		static void load_mime_types();																	//	Load MIME types in a map

		//	Parser
		static int	brackets(std::string & str);
		static int	parse_path(std::string & firstPart, std::string & str, bool isFile, bool check_path );
		static int	parse_body_size(std::string & str);
		static int	parse_errors(const std::string & firstPart, const std::string & secondPart);
		static int	parse_errors(const std::string & firstPart, const std::string & secondPart, VServer & VServ);
		static int	parse_errors(const std::string & firstPart, const std::string & secondPart, Location & Loc);
		static int	parse_autoindex(std::string & str);
		static int	parser_location(std::ifstream & infile, std::string & line, VServer & VServ);
		static int	parser_vserver(std::ifstream & infile, std::string & line);
		static int	parse_global(std::ifstream & infile, std::string & line);
		
	public:

		//	Variables
		static Timer								timer;												//	Class to obtain time and date related data
		static std::string							program_path;										//	Path of the executable
		static std::map <int, std::string>			error_codes;										//	Error codes in a map
		static std::map <std::string, std::string>	mime_types;											//	MIME types in a map
		static std::map <std::string, std::string>	global;												//	Global settings in a map
		static std::vector <VServer>				vserver;											//	V-Servers in a vector
		static std::vector <std::string>			config;												//	Configuration file in a vector
		static std::string							config_path;										//	Path of the default configuration file
		static bool									config_displayed;									//	Is the log or the settings displayed
		static size_t								config_index;										//	Current index of the settings
		static size_t								log_index;											//	Current index of the main log
		static bool									autolog;											//	Auto-Scroll logs
		static bool									check_only;											//	Check the config file, but don't start the server
		static bool									status;												//	Status of the server (On/Off)
		static int									current_vserver;									//	Current selected V-Server (-1 = None)
		static int									terminate;											//	Flag the program to exit with the value in terminate (the default value of -1 don't exit)

		//	Load
		static void			load();																		//	Load the default configuration file
		static void			load(const std::string & File);												//	Load a configuration file
		static void			load_args(int argc, char **argv);											//	Load a configuration file based on the defined arguments

		//	Global
		static std::string	get(const std::string & Key);												//	Get a Value from a Key
		static void			set(const std::string & Key, const std::string & Value);					//	Add or modify a Key - Value
		static void			add(const std::string & Key, const std::string & Value) { set(Key, Value); }//	Alias for 'set'
		static void			del(const std::string & Key);												//	Delete a Key - Value
		static void			clear();																	//	Delete all Keys and his Values

		//	V-Server
		static void			set(VServer & VServ);														//	Add or modify a V-Server
		static void			add(VServer & VServ) { set(VServ); }										//	Alias for 'set'
		static void			del(const VServer & VServ);													//	Delete a V-Server
	
		//	Utils
		static std::string	programPath();																//	Get the path of the executable
		static int			createPath(const std::string & path);										//	Create a path

};
