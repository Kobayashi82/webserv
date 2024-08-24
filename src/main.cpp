/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/24 18:16:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

//  *       service nginx reload

//	*		nc 127.0.0.1 8081	-	telnet 127.0.0.1 8081
//	*		curl -v http://localhost:8081/

//	*		siege -c 255 -t 10S 127.0.0.1:8081			135.000 transactions is a good measure

//	TODO	Manage multi-uploads				(add ID to event que indique nombre de archivo y si es de lectura o escritura)

//	TODO	Read/write functions for epoll
//	TODO	Cache

//	TODO	Process_requested_path

//	TODO	Security for server and cookies

//	TODO	Variables $request_uri $uri

//  Entry point
int main(int argc, char **argv) {
	
    Settings::load_args(argc, argv);

	Log::start_mutex();
	Log::start(); Display::start();

	Net::epoll__create(); Net::socket_create_all();

    while (Display::isTerminate() == -1) {
		Net::epoll_events();
	}
	
	Net::epoll_close();
	Net::socket_close_all();

	Log::stop(); Display::stop();
	Log::release_mutex();

	Display::disableRawMode();
	Settings::clear();

    return (Settings::terminate);
}
