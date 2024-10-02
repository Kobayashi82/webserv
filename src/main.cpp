/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/03 00:03:40 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"
#include "Display.hpp"
#include "Thread.hpp"
#include "Settings.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Communication.hpp"

#pragma region Information

	//	*		curl -v http://127.0.0.1:8081/
	//	*		curl --resolve "webserv:8081:127.0.0.1"  http://webserv:8081
	//	*		siege -b -c 255 -t 10S 127.0.0.1:8081			-	stress test
	//	*		ps --ppid $(pgrep webserv) -o pid,stat,cmd		-	check zombie processes

	//	TODO	Optimize

	//	*		CGI (POST, PUT, DELETE)
	//	*		CGI (php, py)
	//	*		CGI (directory)
	//	*		Web (session manager and cookies)
	//	*		Web (error pages)
	//	*		Web (php, py)
	//	*		Web (directory)
	//	*		Web (uploads)


	//	Es posible que, aunque se añadan y eliminen datos dinámicamente en tu programa (como en contenedores u otros objetos dinámicos),
	//	la memoria se observe que sigue reflejando un uso elevado, incluso después de que los datos hayan sido eliminados.

	//	Esto puede ocurrir por varias razones:

	//	Fragmentación de memoria: El sistema operativo o el gestor de memoria del programa puede no liberar la memoria inmediatamente o de forma eficiente,
	//	lo que puede llevar a que ciertas porciones de memoria no sean devueltas al sistema aunque ya no se estén utilizando.

	//	Retención de memoria por el heap: Cuando liberas memoria en C++, el runtime puede decidir no devolverla inmediatamente al sistema operativo, sino reservarla para futuros usos.
	//	Esto puede dar la impresión de que tu programa sigue utilizando más memoria de la que realmente está ocupando con datos activos.

	//	Buffers y cachés internos: Algunas librerías o el propio sistema operativo podrían estar utilizando memoria para cachés o buffers internos,
	//	lo que puede hacer que la memoria usada no disminuya inmediatamente cuando eliminas datos.

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

		Log::exec_logrot(Utils::fullpath(Settings::program_path + "/.logrotate.cfg"));
		
		return (Settings::terminate);
	}

#pragma endregion
