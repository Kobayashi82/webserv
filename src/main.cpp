/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/06 02:21:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

//  *       service nginx reload


//  TODO    Parse mimo.types
//  TODO    Parse error codes

//  TODO    No identifier
//  TODO    No Repeat
//  TODO    client_max_body_size not valid
//  TODO    port not valid
//  TODO    error_page must be numeric and between a range
//  TODO    Key but no value and (key != { } && key != server_name && key != internal)
//  TODO    If root dont exist and
//  TODO    If log create folder fail or config create folder fail
//  TODO    If root + *.html dont exist
//  TODO    alias dont exist
//  TODO    limit_except must be valid (deny and return too)

//	TODO	Send/Received data
//	TODO	Active conections

//  Entry point
int main(int argc, char **argv) {
    Settings::load_args(argc, argv); if (Settings::terminate != -1) return (Settings::terminate);
	
    Display::enableRawMode();
    while (Settings::terminate == -1) { Display::Input(); usleep(10000); }
	Log::log_access("WebServ 1.0 closed successfully");
    Display::disableRawMode(); Settings::clear();
    return (Settings::terminate);
}
