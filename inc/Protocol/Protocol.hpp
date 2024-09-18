/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Protocol.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 12:49:42 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 19:52:05 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <map>																							//	For std::map container

#pragma region Intermediary

	struct EventInfo;																					//	Forward declaration of EventInfo
	class Intermediary {

		public:

			//	Methods
			static std::map<std::string, std::string>	response_data(EventInfo * event);

	};

#pragma endregion

//	-------------------------------------------------------------
//	AQUI VAN LAS CLASES Y COSAS DE "REQUEST.CPP" Y "RESPONSE.CPP"
//	-------------------------------------------------------------



//	Protocol 				HTTP/1.1
//	Type					error, redirection, file, directory, cgi
//	Path					ruta de la redirección, archivo, carpeta o cgi	(incluye el "Location" en caso de redireccion)	(incluye el "Allow" en caso de error 405 Method Not Allowd)
//	Code					código de la respuesta (200, 404, etc...)
//	CodeInfo				descripción del código
//	Date					fecha actual (Tue, 12 Sep 2024 15:44:18 GMT)
//	Server					Webserv/1.0
//	X-Content-Type-Options	nosniff
//	Connection:				keep-alive (close solamente si el cliente lo solicita)
//	Content-Type			tipo de contendio (text/html; charset=UTF-8)  charset=UTF-8 solo si el tipo es "text/"
//	Content-Length:			tamaño del cuerpo del mensaje en bytes



//	Protocol:				HTTP/1.1
//	Code:					200
//	Code_Info:				OK

//	Date:					Tue, 12 Sep 2024 15:44:18 GMT
//	Server:					Webserv/1.0
//	X-Content-Type-Options:	nosniff
//	Connection:				close or keep-alive
//	Content-Type:			text/html; charset=UTF-8
//	Content-Length:			1234

//	Location:				http://www.new-url.com/		(para redirecciones)
//	Allow:					GET, POST					(para metodos no allowed)
//	Cache-Control
//	Last-Modified

//?	ARCHIVO ESTATICO
//?	----------------
//	HTTP/1.1 200 OK
//	Date: Tue, 12 Sep 2024 15:44:18 GMT
//	Server: Webserv/1.0
//	Content-Type: text/html; charset=UTF-8
//	Content-Length: 1234
//	Connection: close

//	<HTML content goes here...>

//?	ERROR
//?	-----
//	HTTP/1.1 404 Not Found
//	Date: Tue, 12 Sep 2024 15:44:18 GMT
//	Server: Webserv/1.0
//	Content-Type: text/html; charset=UTF-8
//	Content-Length: 88
//	Connection: close

//	<html><body><h1>404 Not Found</h1><p>The requested resource could not be found.</p></body></html>

//?	REDIRECCION
//?	-----------
//	HTTP/1.1 301 Moved Permanently
//	Date: Tue, 12 Sep 2024 15:44:18 GMT
//	Server: MyCustomServer/1.0
//	Location: http://www.new-url.com/
//	Content-Length: 0
//	Connection: close

//?	DIRECTORIO
//?	----------
//	HTTP/1.1 200 OK
//	Date: Tue, 12 Sep 2024 15:44:18 GMT
//	Server: MyCustomServer/1.0
//	Content-Type: text/html; charset=UTF-8
//	Content-Length: 456
//	Connection: close

//	<html><body><h1>Index of /dir</h1><ul><li><a href="file1.txt">file1.txt</a></li><li><a href="file2.jpg">file2.jpg</a></li></ul></body></html>

//?	PROTOCOLO NO SOPORTADO (HTTP/2.0)
//?	---------------------------------
//	HTTP/1.1 505 HTTP Version Not Supported
//	Content-Type: text/plain
//	Content-Length: 30

//	HTTP/2.0 Protocol Not Supported

//?	IP DENEGADA
//?	-----------
//	HTTP/1.1 403 Forbidden
//	Date: Fri, 13 Sep 2024 15:44:18 GMT
//	Server: Webserv/1.0
//	Content-Type: text/html; charset=UTF-8
//	Content-Length: 1234

//	<html>
//	<head><title>403 Forbidden</title></head>
//	<body>
//	<h1>403 Forbidden</h1>
//	<p>You don't have permission to access / on this server.</p>
//	</body>
//	</html>