/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Settings.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 12:14:05 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/28 19:23:31 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "VServer.hpp"
#include "Timer.hpp"
#include "Log.hpp"
#include <unistd.h>
#include <limits.h>
#include <cstdio>
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
		static Timer								timer;


		//	Load
		static void load();
		static void load(const std::string & File, bool isReisRegen = false);

		//	Global
		static std::string get(const std::string & Key);
		static void set(const std::string & Key, const std::string & Value);
		static void add(const std::string & Key, const std::string & Value) { set(Key, Value); }
		static void del(const std::string & Key);

		//	VServer
		static void set(const VServer & VServ);
		static void add(const VServer & VServ) { set(VServ); }
		static void del(const VServer & VServ);

		static std::string ProgramPath();
		static void create_path(const std::string & path);
		static void clear();

};
