/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/23 13:31:11 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

//  *       service nginx reload

//	*		nc 127.0.0.1 8081	-	telnet 127.0.0.1 8081
//	*		curl -v http://localhost:8081/

//	*		siege -c 255 -t 10S 127.0.0.1:8081			135.000 transactions is a good measure

//	TODO	get_ip_range no añadir 0 y broadcast solo cuando range
//	TODO	Interface tabs

//	TODO	Write_to_file
//	TODO	Read_from_file
//	TODO	Manage multi-uploads	(add ID to event que indique nombre de archivo y si es de lectura o escritura)

//	TODO	Pasar interfaz a un hilo (no queda otra)
//	TODO	Mejor interfaz para -i (y en hilo tambien)

//	TODO	keepalive_timeout 75;				>= 0 && <= 120 		0 = disabled			defaults to 30
//	TODO	keepalive_requests 1000;			> 0 && <= 5000								defaults to 500

//	TODO	total conections and bandwidth with a fixed size

//  Entry point
int main(int argc, char **argv) {
    Settings::load_args(argc, argv);

	Log::start(); Display::start();

	Net::epoll__create();
	Net::socket_create_all();

    while (Settings::terminate == -1) {
		if (Net::epoll_events() == -1) { Log::log(RD "Error de epoll_wait" NC, Log::BOTH_ERROR); Settings::terminate = 1; break; }
	}
	
	Net::epoll_close(); Net::socket_close_all();

	Log::stop(); Display::stop();

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