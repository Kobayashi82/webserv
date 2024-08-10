/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/10 23:08:02 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

//  *       service nginx reload

//  TODO    No Repeat
//  TODO    Key but no value and (key != { } && key != server_name && key != internal)
//  TODO    limit_except must be valid (deny and return too)

//	TODO	Send/Received data
//	TODO	Active conections

//  Entry point
int main(int argc, char **argv) {
    Settings::load_args(argc, argv);
	
    while (Settings::terminate == -1) {
		Display::Input(); usleep(10000);
	}

	if (!Settings::check_only && !Display::RawModeDisabled && !Display::ForceRawModeDisabled) {
		usleep(100000); std::cout.flush(); std::cout.clear(); Display::maxFails = 10; Display::failCount = 0; Display::drawing = false;
		Log::log_access("WebServ 1.0 closed successfully");
		Display::disableRawMode();
	} else std::cout << CSHOW << std::endl;

	Settings::clear();
    return (Settings::terminate);
}
