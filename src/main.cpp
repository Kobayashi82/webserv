/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/29 15:06:07 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

//  parse mimo.types
//  service nginx reload

void printSettings() {
    // Imprimir Settings::global
    std::cout << "Global Settings:" << std::endl << std::endl;
    for (std::map<std::string, std::string>::iterator it = Settings::global.begin(); it != Settings::global.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << std::endl;

    // Imprimir cada VServer
    for (size_t i = 0; i < Settings::vserver.size(); ++i) {
        const VServer& vserver = Settings::vserver[i];
        std::cout << "VServer " << i + 1 << ":" << std::endl << std::endl;
        for (std::map<std::string, std::string>::const_iterator it = vserver.vserver.begin(); it != vserver.vserver.end(); ++it) {
            std::cout << it->first << ": " << it->second << std::endl;
        }
        std::cout << std::endl;

        // Imprimir cada Location dentro del VServer
        for (size_t j = 0; j < vserver.location.size(); ++j) {
            const Location& location = vserver.location[j];
            std::cout << "  Location " << j + 1 << ":" << std::endl << std::endl;
            for (std::map<std::string, std::string>::const_iterator it = location.location.begin(); it != location.location.end(); ++it) {
                std::cout << "    " << it->first << ": " << it->second << std::endl;
            }
            std::cout << std::endl;
        }
    }
}

void settings_test(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "-t")) {                                                                                      //  Test default settings
        Settings::check_only = true;
        Settings::load();
		Settings::terminate = 0;
    } else if (argc == 3 && (!strcmp(argv[1], "-t") || !strcmp(argv[2], "-t"))) {                                                   //  Test indicated settings
    	Settings::check_only = true;
        if (!strcmp(argv[1], "-t") && !strcmp(argv[2], "-t")) {
            std::cout << std::endl << RD "\t\t     Test config file" << std::endl << std::endl
                      << C "\tUsage: " Y "./webserv -t [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
        }
        else if (strcmp(argv[1], "-t")) Settings::load(argv[1]);
        else if (strcmp(argv[2], "-t")) Settings::load(argv[2]);
		Settings::terminate = 0;
    } else if (argc > 2) {
		std::cout << std::endl << RD "\t     Incorrect number of arguments" << std::endl << std::endl
				  << C "\tUsage: " Y "./webserv [" B "Opional " G "settings file" Y "]" NC << std::endl << std::endl;
		Settings::terminate = 1;
    } else {
        if (argc == 1) Settings::load(); else Settings::load(argv[1]);
        if (Settings::vserver.size() == 0) {
            if (Settings::loaded_ok) Log::log_error("There are no virtual servers in the configuration file", NULL, true);
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
    printSettings();
    Settings::clear();
    return (Settings::terminate);
}
