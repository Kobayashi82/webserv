/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/21 19:22:11 by vzurera-         ###   ########.fr       */
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

//	TODO	./webserv -i with siege overloaded with logs
//			Buffer con los últimos 100 logs y que se impriman cada x tiempo o cuando se llenen... probaré así.
//			En el main the logs mantener un timer o algo. Puede ser de 200ms o algo asi

//	TODO	Hay transacciones fantasmas. Pueden ser que se repitan?
//	TODO	Si tiene que crear el archivo de log se producen transacciones fallidas. WTF!!

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
		if (Display::signal_check())	break;
		if (Epoll::events())			break;
	}
	
	Epoll::close();
	Socket::close();
	Event::remove();

	Log::stop(); Display::stop(); Log::release_mutex(); Display::disableRawMode();

	Log::exec_logrot(Settings::program_path + ".logrotate.cfg");
	
	return (Settings::terminate);
}
