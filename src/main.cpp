/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/30 16:08:35 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"
#include "Display.hpp"
#include "Settings.hpp"
#include "Socket.hpp"
#include "Communication.hpp"
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
	//	TODO	General errors overhaul
	//	TODO	Cache not working as it should
	//	TODO	Optimized stat calls 
	//	TODO	La puta se cierra cuando pongo un video, a veces

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

#pragma region Reload

	void reload(int argc, char **argv) {
		Epoll::close();
		Socket::close();
		Event::remove();

		Log::stop(); Display::stop(); Log::release_mutex(); Display::disableRawMode();

		Communication::total_clients = 0;
		Communication::write_bytes = 0;
		Communication::read_bytes = 0;

		Settings::clear(true);
		Settings::load_args(argc, argv);

		Display::signal_handler();
		Log::start(); Display::start();
				
		Epoll::create();
		Socket::create(true);

		usleep(10000); Display::update();
	}

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
			if (Display::isTerminate() == 4) reload(argc, argv);
		}
		
		Epoll::close();
		Socket::close();
		Event::remove();

		Log::stop(); Display::stop(); Log::release_mutex(); Display::disableRawMode();

		Log::exec_logrot(Settings::program_path + ".logrotate.cfg");
		
		return (Settings::terminate);
	}

#pragma endregion
