/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 21:32:23 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"
#include "Display.hpp"
#include "Settings.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Thread.hpp"

//	*		nc 127.0.0.1 8081	-	telnet 127.0.0.1 8081	-	curl -v http://127.0.0.1:8081/
//	*		siege -b -c 255 -t 10S 127.0.0.1:8081

//	?		Non-bloquing fd

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

	Display::signal_handler();
	Log::start(); Display::start();
	
	Epoll::create();
	Socket::create();

	usleep(10000); Display::update();

	while (Display::isTerminate() == -1) {
		if (Display::signal_check()) break;
		if (Epoll::events()) {
			break;
		}
	}
	
	Epoll::close();
	Socket::close();
	Event::remove();

	Log::stop(); Display::stop(); Log::release_mutex(); Display::disableRawMode();

	Log::exec_logrot(Settings::program_path + ".logrotate.cfg");
	
	return (Settings::terminate);
}
