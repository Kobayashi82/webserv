/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/17 16:31:39 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"
#include "Display.hpp"
#include "Settings.hpp"
#include "Net.hpp"

//	*		nc 127.0.0.1 8081	-	telnet 127.0.0.1 8081	-	curl -v http://127.0.0.1:8081/
//	*		siege -b -c 255 -t 10S 127.0.0.1:8081

//	?		Non-bloquing fd

//			CONNECTIONS
//	TODO	Vserver take control if other is disabled
//	TODO	EPOLL fail error (create or events)

//			DISPLAY
//	TODO	./webserv -i with siege overloaded with logs

//			INTERMEDIARY
//	TODO	Intermediario
//	TODO	data of vserver/location in event
//	TODO	Index usa index.html por defecto
//	TODO	Update resource path with alias or any modified path before cgi

//			COMUNICATIONS
//	TODO	Transfer big files
//	TODO	Comunications time-out

//			I MA NOT DOING THESE
//	*		Process request
//	*		Generate response
//	*		CGI   (POST, PUT, PATCH, DELETE)
//	*		CGI   (php, py)
//	*		CGI   (directory)
//	*		CGI   (uploads)
//	*		Web (error pages)
//	*		Web (session manager and cookies)
//	*		Web (uploads)
//	*		Web (CGI)
//	*		Web (directory)

//  Entry point
int main(int argc, char **argv) {
	Settings::load_args(argc, argv);

	Log::start(); Display::start();
	
	Net::epoll__create();
	Net::socket_create_all();

	usleep(10000); Display::update();

	while (Display::isTerminate() == -1) {
		Net::epoll_events();
	}
	
	Net::epoll_close();
	Net::socket_close_all();
	Net::remove_events();

	Log::stop(); Display::stop(); Log::release_mutex(); Display::disableRawMode();

	Log::exec_logrot(Settings::program_path + ".logrotate.cfg");
	
	return (Settings::terminate);
}
