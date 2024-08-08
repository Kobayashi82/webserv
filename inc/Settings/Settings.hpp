/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:14:05 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 15:52:38 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Colors.hpp"
#include "VServer.hpp"
#include "Timer.hpp"
#include "Log.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <sstream>																						//	For std::ostringstream to format strings
#include <fstream>																						//	For file stream classes like std::ifstream, std::ofstream

#include <algorithm>																					//	For algorithms like std::sort, std::find, etc.
#include <vector>																						//	For the std::vector container
#include <deque>																						//	For the std::deque container
#include <map>																							//	For the std::map container

#include <cstdio>																						//	For C-style input and output functions
#include <cstring>																						//	For C-style string functions like strcmp()

#include <unistd.h>																						//	For functions like sleep, fork, etc.
#include <limits.h>																						//	For defining the characteristics of fundamental types
#include <sys/stat.h>																					//	For data returned by the functions fstat(), lstat(), and stat()
#include <sys/types.h>																					//	For data types used in system calls


class Settings {

	public:

		//	Variables
		static Timer								timer;												//	Class to obtain time and date related data
		static std::string							program_path;										//	Path of the executable
		static std::string							config_path;										//	Path of the default configuration file
		static std::map <int, std::string>			error_codes;										//	Error codes in a map
		static std::map <std::string, std::string>	global;												//	Global settings in a map
		static std::vector <VServer>				vserver;											//	V-Servers in a vector
		static std::vector <std::string>			config;												//	Configuration file in a vector
		static bool									config_displayed;									//	Is the log or the settings displayed
		static size_t								config_index;										//	Current index of the settings
		static size_t								log_index;											//	Current index of the main log
		static bool									autolog;											//	Auto-Scroll logs
		static bool									check_only;											//	Check the config file, but don't start the server
		static bool									loaded_ok;											//	The config file loaded successfully (but may contains errors)
		static bool									status;												//	Status of the server (On/Off)
		static bool									BadConfig;											//	Indicate if there are errors in the config file
		static int									current_vserver;									//	Current selected V-Server (-1 = None)
		static int									terminate;											//	Flag the program to exit with the value in terminate (the default value of -1 don't exit)

		//	Load
		static void			load();																		//	Load the default configuration file
		static void			load(const std::string & File, bool isReisRegen = false);					//	Load a configuration file
		static void			load_args(int argc, char **argv);											//	Load a configuration file based on the defined arguments

		//	Global
		static std::string	get(const std::string & Key);												//	Get a Value from a Key
		static void			set(const std::string & Key, const std::string & Value);					//	Add or modify a Key - Value
		static void			add(const std::string & Key, const std::string & Value) { set(Key, Value); }//	Alias for 'set'
		static void			del(const std::string & Key);												//	Delete a Key - Value
		static void			clear();																	//	Delete all Keys and his Values

		//	VServer
		static void			set(VServer & VServ);														//	Add or modify a V-Server
		static void			add(VServer & VServ) { set(VServ); }										//	Alias for 'set'
		static void			del(const VServer & VServ);													//	Delete a V-Server

		//	Utils
		static std::string	programPath();																//	Get the path of the executable
		static int			createPath(const std::string & path);										//	Create a path
		static std::string	itos(long number);															//	Convert a number to a string

};

	void load_error_codes();																			//	Load error codes in a map
