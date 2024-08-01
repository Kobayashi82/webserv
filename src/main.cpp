/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/02 01:04:58 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

//  *       service nginx reload

//  TODO    Terminal interface

//  TODO    Parse mimo.types
//  TODO    Parse error codes

//  TODO    No identifier
//  TODO    No Repeat
//  TODO    Count { }
//  TODO    client_max_body_size not valid
//  TODO    port not valid
//  TODO    error_page must be numeric and between a range
//  TODO    Key but no value and (key != { } && key != server_name && key != internal)
//  TODO    If root dont exist and
//  TODO    If log create folder fail or config create folder fail
//  TODO    If root + *.html dont exist
//  TODO    alias dont exist
//  TODO    limit_except must be valid (deny and return too)
//  TODO    Log all errors
//  TODO    If errors, dont load and log

//  Entry point
int main(int argc, char **argv) {
    Settings::load_args(argc, argv); if (Settings::terminate != -1) return (Settings::terminate);

    //setTerminalSize(30, 100);
    Display::enableRawMode();
    Display::Output();
    while (Settings::terminate == -1) {
        Display::Input(); usleep(10000);
    } Display::disableRawMode();

    Settings::print();
    Log::log_access("Hola caracola", &Settings::vserver[0]);
    std::cout << std::endl << G "VSERVER (" << Settings::vserver[0].get("server_name") << ")" NC << std::endl << std::endl;
    std::cout << Log::both(&Settings::vserver[0]) << std::endl;

    std::cout << std::endl << G "ACCESS " NC << std::endl << std::endl;
    std::cout << Log::access() << std::endl;
    std::cout << std::endl << RD "ERROR " NC << std::endl << std::endl;
    std::cout << Log::error() << std::endl;
    std::cout << std::endl << G "ACCESS" NC "/" RD "ERROR " NC << std::endl << std::endl;
    std::cout << Log::both() << std::endl;
    Settings::clear();
    return (Settings::terminate);
}
