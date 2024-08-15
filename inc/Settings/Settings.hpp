/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:14:05 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/15 19:04:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Colors.hpp"
#include "Display.hpp"
#include "Utils.hpp"
#include "Timer.hpp"
#include "VServer.hpp"

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
		static bool									BadConfig;											//	Indicate if there are errors in the config file
		static int									line_count;											//	Number of the current line of the configuration file (use to indicate the line of an error in the configuration file)
		static int									bracket_lvl;										//	Level of the bracket (use to parse the configuration file)

		//	Errors and MIMEs
		static void load_error_codes();																	//	Load error codes in a map
		static void load_mime_types();																	//	Load MIME types in a map

		//	Parser
		static int	brackets(std::string & str);
		static int	parse_path(const std::string & firstPart, std::string & str, bool isFile, bool check_path, bool check_write);
		static int	parse_body_size(std::string & str);
		static int	parse_errors(const std::string & firstPart, const std::string & secondPart);
		static int	parse_errors(const std::string & firstPart, const std::string & secondPart, VServer & VServ);
		static int	parse_errors(const std::string & firstPart, const std::string & secondPart, Location & Loc);
		static int	parse_autoindex(std::string & str);
		static int	parse_index(std::string & str);
		static int	parse_listen(std::string & str, VServer & VServ);
		static int	parse_return(std::string & str);
		static int	parse_alias(std::string & firstPart, std::string & str);
		static int	parse_try_files(std::string & str);
		static int	parse_allow(std::string & str);
		static int	parse_deny(std::string & str);
		static int	parse_limit_except(std::string & str);
		static int	parse_location(std::string & str);
		static int	parse_cgi(const std::string & firstPart, const std::string & secondPart);
		static int	parse_cgi(const std::string & firstPart, const std::string & secondPart, VServer & VServ);
		static int	parse_cgi(const std::string & firstPart, const std::string & secondPart, Location & Loc);

		static int	parser_method(std::ifstream & infile, std::string & line, VServer VServ, Location & Loc);
		static int	parser_location(std::ifstream & infile, std::string & line, VServer & VServ);
		static int	parser_vserver(std::ifstream & infile, std::string & line);
		static int	parse_global(std::ifstream & infile, std::string & line);
		
	public:

		//	Variables
		static Timer								timer;												//	Class to obtain time and date related data
		static std::string							program_path;										//	Path of the executable
		static std::string							config_path;										//	Path of the default configuration file

		static VServer								global;												//	Global settings in a vector
		static std::vector <VServer>				vserver;											//	V-Servers in a vector

		static std::map <int, std::string>			error_codes;										//	Error codes in a map
		static std::map <std::string, std::string>	mime_types;											//	MIME types in a map

		static bool									check_only;											//	Check the config file, but don't start the server
		static bool									loaded_ok;											//	The config file loaded successfully (but may contains errors)
		static int									current_vserver;									//	Current selected V-Server (-1 = None)
		static int									terminate;											//	Flag the program to exit with the value in terminate (the default value of -1 don't exit)

		//	Load
		static void			load();																		//	Load the default configuration file
		static void			load(const std::string & File);												//	Load a configuration file
		static void			load_args(int argc, char **argv);											//	Load a configuration file based on the defined arguments

		//	Global
		static void			clear(bool reset = false);															//	Delete all Keys, his Values and optionally reset config data

		//	V-Server
		static void			set(VServer & VServ);														//	Add or modify a V-Server
		static void			add(VServer & VServ);														//	Alias for 'set'
		static void			del(const VServer & VServ);													//	Delete a V-Server
		static void			vserver_clear();															//	Delete all V-Servers

};
