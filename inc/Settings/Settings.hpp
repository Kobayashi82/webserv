/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:14:05 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/29 20:53:31 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Colors.hpp"
#include "VServer.hpp"
#include "Timer.hpp"
#include "Log.hpp"
#include <unistd.h>
#include <limits.h>
#include <cstdio>
#include <iostream>
#include <cstring>			//	For strcmp()
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>

class Settings {

	public:

		//	Variables
		static std::map <std::string, std::string>	global;
		static std::vector <VServer>				vserver;
		static std::string							program_path;
		static std::string							config_path;
		static int									terminate;
		static int									bracket_lvl;
		static bool									check_only;
		static bool									loaded_ok;
		static Timer								timer;


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
		static void			set(const VServer & VServ);
		static void			add(const VServer & VServ) { set(VServ); }
		static void			del(const VServer & VServ);

		//	Utils
		static std::string	programPath();
		static void			createPath(const std::string & path);
		static void			print();

};
