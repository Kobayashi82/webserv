/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/29 15:03:47 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"

static void log(const std::string & str, std::string path = "") {
	if (Settings::check_only) std::cout << str << std::endl;
	else {
		if (path != "" && path[0] != '/') path = Settings::program_path + path;
		Settings::create_path(path);
		std::ofstream outfile;
        outfile.open(path.c_str(), std::ios_base::app);
        if (outfile.is_open()) {
        	outfile << "[" << Settings::timer.current_date() << " " << Settings::timer.current_time() << "] - " << str << std::endl;
        	outfile.close();
        }
	}
}

void Log::log_access(const std::string & str, const VServer * VServ, bool Default) {
	if (Default) log(str, Settings::program_path + "logs/access.log");
	else if (VServ && VServ->get("access_log") != "") log(str, VServ->get("access_log"));
	else log(str, Settings::get("access_log"));
}

void Log::log_error(const std::string & str, const VServer * VServ, bool Default) {
	if (Default) log(str, Settings::program_path + "logs/error.log");
	else if (VServ && VServ->get("error_log") != "") log(str, VServ->get("error_log"));
	else log(str, Settings::get("error_log"));
}
