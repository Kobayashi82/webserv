/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/20 22:23:41 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

//  *       service nginx reload

//	*		nc 127.0.0.1 8081	-	telnet 127.0.0.1 8081
//	*		curl -v http://localhost:8081/

//	*		siege -c 255 -t 10S 127.0.0.1:8081

//	TODO	get_ip_range no añadir 0 y broadcast solo cuando range
//	TODO	Interface tabs

//	TODO	check_logs no error if size < 11 (creo)
//	TODO	Algoritmo para detectar dia en los logs y que sea mas rapido. Para eliminar log_days
//	TODO	Pasar interfaz a un hilo (no queda otra)
//	TODO	Mejor interfaz para -i (y en hilo tambien)

//	TODO	keepalive_timeout 75;				>= 0 && <= 120 		0 = disabled			defaults to 30
//	TODO	keepalive_requests 1000;			> 0 && <= 5000								defaults to 500

//  Entry point
int main(int argc, char **argv) {
    Settings::load_args(argc, argv);

	Net::epoll__create();
	Net::socket_create_all();

    while (Settings::terminate == -1) {
		if (Net::epoll_events() == -1) { Log::log_error(RD "Error de epoll_wait" NC); break; }
		if (Display::Resized) { if (Display::drawing) Display::redraw = true; else Display::Output(); }
		Display::Input();
	}

	Net::epoll_close(); Net::socket_close_all(); //Net::clientsClose();

	if (!Settings::check_only && !Display::RawModeDisabled && !Display::ForceRawModeDisabled) {
		usleep(100000); std::cout.flush(); std::cout.clear(); Display::maxFails = 10; Display::failCount = 0; Display::drawing = false;
		Log::log_access(G "WebServ 1.0 closed successfully");
		Display::disableRawMode();
	} else std::cout << CSHOW << std::endl;

	Settings::clear();
    return (Settings::terminate);
}

//	GLOBAL

//	✓	access_log										/var/log/nginx/access.log;
//	✓	error_log										error.log;
//	✓	client_max_body_size							10M;
//	✓	error_page 404 500								=200 /about/index.html;
//	✓	autoindex										on;

//	SERVER	(server {)

//	✓	listen											80;													ip:port
//	✓	server_name										example.com www.example.com;
//	✓	root											/mnt/c/www/html/example.com;
//	✓	index											index.html index.htm index.php;

//		LOCATION
//
//	✓		location 									= /404.html {
//	✓		internal;
//	✓		alias										/mnt/c/www/html/error_pages/404.html;
//	✓		try_files									$uri $uri/ file1.html /file2.html =404;								$uri $uri/ /file.html;
//	✓		return										301 https://example.com$request_uri;			$request_uri;
//	✓		limit_except								GET POST {											Server y Location
//	✓			deny									all;												Global, Server y Location
//	✓			allow									192.168.1.0/24;		 192.168.1.0/255.255.255.0;		Global, Server y Location
//	✓			return									405 /405.html;

//	✓		http puede estar, pero se ignora y solo puede estar antes del primer {

//	✓	uploads /path
//	✓	cgi .py /usr/bin/python3;
//	✓	cgi .php .otro /usr/bin/php;

//	Variables $request_uri $uri
//	crear clase de variables con map $request_uri $uri