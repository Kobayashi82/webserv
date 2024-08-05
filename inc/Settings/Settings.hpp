/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:14:05 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/05 23:07:26 by vzurera-         ###   ########.fr       */
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
		static std::map <std::string, std::string>	global;
		static std::vector <VServer>				vserver;
		static std::string							program_path;
		static std::string							config_path;
		static std::vector <std::string>			config;												//	Settings in a vector
		static bool									config_displayed;									//	Is the log or the settings displayed
		static size_t								config_index;										//	Current index of the settings
		static size_t								log_index;											//	Current index of main log
		static bool									autolog;											//	Auto scroll logs
		static int									terminate;
		static int									bracket_lvl;
		static bool									check_only;
		static bool									loaded_ok;
		static bool									errors;
		static bool									status;
		static Timer								timer;
		static int									current_vserver;

		//	Load
		static void			load();
		static void			load(const std::string & File, bool isReisRegen = false);
		static void			load_args(int argc, char **argv);

		//	Global
		static std::string	get(const std::string & Key);
		static void			set(const std::string & Key, const std::string & Value);
		static void			add(const std::string & Key, const std::string & Value) { set(Key, Value); }
		static void			del(const std::string & Key);
		static void			clear();

		//	VServer
		static void			set(VServer & VServ);
		static void			add(VServer & VServ) { set(VServ); }
		static void			del(const VServer & VServ);

		//	Utils
		static std::string	programPath();
		static int			createPath(const std::string & path);
		static void			print();

};
