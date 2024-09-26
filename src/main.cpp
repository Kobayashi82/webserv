/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/26 22:00:03 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"
#include "Display.hpp"
#include "Settings.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Thread.hpp"

#pragma region Information

	//	*		nc 127.0.0.1 8081
	//	*		telnet 127.0.0.1 8081
	//	*		curl -v http://127.0.0.1:8081/

	//	*		siege -b -c 255 -t 10S 127.0.0.1:8081			-	stress test
	//	*		ps --ppid $(pgrep webserv) -o pid,stat,cmd		-	check zombie processes

	//	TODO	./webserv -i with siege overloaded with logs
	//			Buffer con los últimos 100 logs y que se impriman cada x tiempo o cuando se llenen... probaré así.
	//			En el main the logs mantener un timer o algo. Puede ser de 200ms o algo asi

	//	TODO	Check events timeout from another thread (non-blocking cant be done with cgi)
	//	TODO	Better error
	//	TODO	General errors overhaul
	//	TODO	Log to file better line

	//	TODO	Intermediary
	//	TODO	Documentation

	//	*		CGI	(POST, PUT, PATCH, DELETE)
	//	*		CGI	(php, py)
	//	*		CGI	(directory)
	//	*		CGI	(uploads)
	//	*		Web	(session manager and cookies)
	//	*		Web	(error pages)
	//	*		Web	(php, py)
	//	*		Web	(directory)
	//	*		Web	(uploads)

#pragma endregion

#pragma region Main

	int main(int argc, char **argv) {
		Settings::load_args(argc, argv);

		Display::signal_handler();
		Log::start(); Display::start();
		
		Epoll::create();
		Socket::create(true);

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

#pragma endregion
