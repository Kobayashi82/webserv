/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/27 21:19:55 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include <cstring>

//  webserv -t      test config file 
//  parser settings
//  parser mimo.types
//  service nginx reload

void settings_test(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "-t")) {                                                                                      //  Test default settings
		std::cout << std::endl << RD "\t     Test default config file" << std::endl << std::endl
				  << C "\tUsage: " Y "./webserv [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
		Settings::terminate = 0;
    } else if (argc == 3 && (!strcmp(argv[1], "-t") || !strcmp(argv[2], "-t"))) {                                                   //  Test indicated settings
		std::cout << std::endl << RD "\t     Test config file" << std::endl << std::endl
				  << C "\tUsage: " Y "./webserv [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
        Settings::terminate = 0;
    } else if (argc > 2) {
		std::cout << std::endl << RD "\t     Incorrect number of arguments" << std::endl << std::endl
				  << C "\tUsage: " Y "./webserv [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
		Settings::terminate = 1;
    } else {
        //  Check default settings, if broken or missing, generate
        //  If argc = 1 use default settings
        //  If argc = 2 validate, overwrite default and use new settings
        Settings::load();
        if (Settings::vserver.size() == 0) {
    		std::cout << std::endl << C "\tCould not start the server, check the file:" << std::endl << std::endl
                      << Y "\t" << Settings::program_path + "logs/error.log" NC << std::endl << std::endl;
            Settings::terminate = 1;
        }
    }
}

int main(int argc, char **argv) {
    settings_test(argc, argv); if (Settings::terminate != -1) return (Settings::terminate);
    
    // enableRawMode();
    // while (Settings::terminate == -1) {
    //     printOutput();
    // } disableRawMode();
    
    Settings::clear();
    return (Settings::terminate);
}
