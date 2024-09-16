/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/16 20:11:47 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"
#include "Display.hpp"
#include "Settings.hpp"
#include "Net.hpp"

//	service nginx reload
//	nc 127.0.0.1 8081	-	telnet 127.0.0.1 8081	-	curl -v http://127.0.0.1:8081/
//	siege -b -c 255 -t 10S 127.0.0.1:8081			135.000 transactions is a good measure

//	 ?		Non-bloquing fd
//	 ?		Log through epoll

//				CONNECTIONS
//	 TODO	Vserver take control if other is disabled
//	 TODO	Si 0.0.0.0 o sin ip, usar any.

//				DISPLAY
//	 TODO	Info on IPs active in vserver
//	 TODO	Log position to first key down need tWo taps
//	 TODO	Global log only log what is not in other logs
//	 TODO	Display add access, error or both to logs
//	 TODO	Positions for access, error and log 
//	 TODO	Better name for vservers
//	 TODO	Reload config button
//	 TODO	./webserv -i with siege overloaded with logs

//			CONFIG
//	 TODO	Method in global, server y location

//			INTERMEDIARY
//	 TODO	Intermediario
//	 TODO	Index usa index.html por defecto
//	 TODO	Update resource path with alias or any modified path before cgi
//	 TODO	If cgi, cant load request first, must analyze header before

//			COMUNICATIONS
//	 TODO	Transfer big files

//			DOCUMENTATION
//	 TODO	Documentation (conexiones, cache, request, response, cgi, methods, cookies and sessions, directories)
//	 TODO	Diagram

//  Entry point
int main(int argc, char **argv) {
	Settings::load_args(argc, argv);

	// Log::log("GET", "/prueba", 100, 1024, "250", "127.0.0.1");
	// Log::log("HEAD", "/home/casa", 200, 1024, "250", "127.0.0.1");
	// Log::log("POST", "/algo", 300, 1024, "250", "127.0.0.1");
	// Log::log("PUT", "/hola/caracola", 400, 1024, "250", "127.0.0.1");
	// Log::log("DELETE", "/everything", 500, 1024, "250", "127.0.0.1");

	Log::start(); Display::start();
	
	Net::epoll__create();																				//	Si falla crear un modo que el servidor no puede ejecutarse, pero se pueden ver los vservers
	Net::socket_create_all();

	usleep(10000); Display::update();

	while (Display::isTerminate() == -1) {
		Net::epoll_events();																			//	Si falla crear un modo que el servidor no puede ejecutarse, pero se pueden ver los vservers
	}
	
	Net::epoll_close();
	Net::socket_close_all();
	Net::remove_events();

	Log::stop(); Display::stop(); Log::release_mutex(); Display::disableRawMode();

	Log::exec_logrot(Settings::program_path + ".logrotate.cfg");
	
	return (Settings::terminate);
}
