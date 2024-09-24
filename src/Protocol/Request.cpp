/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 11:52:00 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/24 19:36:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Protocol.hpp"

#pragma region Parse Request

	//	TODO	data of vserver/location in event
	//	TODO	Index usa index.html por defecto
	//	TODO	Update resource path with alias or any modified path before cgi

	void Protocol::parse_request(EventInfo * event) {
		if (!event) return;

		//								ESTO VA EN PARSE_RESPONSE
		//	--------------------------------------------------------------------------------------
		size_t content_length = 0;
		Utils::stol(event->header_map["Content_length"], content_length);								//	Get 'content_length' in numeric format
		if (event->body_maxsize > 0 && content_length > event->body_maxsize) {							//	If 'content_length' is greater than 'body_maxsize'
			event->header_map["Connection"] = "close";													//	Set the conection to close
			event->header_map["Error"] = "413";															//	Body too big, return error
		}

		//	If (MIME != response type) { }

		// event->method = event->response_map["method"];

		//	--------------------------------------------------------------------------------------

		// event->response_map["method"] = "CGI";
		event->response_map["cgi_path"] = "/usr/bin/php-cgi";
		//event->response_map["cgi_path"] = "/bin/cat";
		//event->response_map["path"] = "file2.html";
		event->response_map["path"] = "index.php";
		// event->method = event->response_map["method"];


		//event->response_map["path"] = "index.php";
		if (event->header_map["Path"] == "/favicon.ico") event->response_map["path"] = "favicon.ico";
		event->response_map["Content-Type"] = "text/html";
		size_t pos = event->response_map["path"].find_last_of('.');
		if (pos != std::string::npos) event->response_map["Content-Type"] = Settings::mime_types[event->response_map["path"].substr(pos + 1)];

		event->response_map["Protocol"] = "HTTP/1.1";
		event->response_map["Connection"] = event->header_map["Connection"];
		event->response_map["code"] = "200";
		event->response_map["code_description"] = Settings::error_codes[Utils::sstol(event->response_map["code"])];
		event->response_map["method"] = "CGI";
		if (event->header_map["Path"] == "/favicon.ico") event->response_map["method"] = "File";
		event->method = event->response_map["method"];

	}

	//	Directivas tienen preferencias de orden

	//	List of VServer that can manage the ip:port of the conection, aka event->socket->IP & port. Must be enabled
	//	Get the VServer that manage the request
	//	Get the location that manage the request
	//	Get the limit_except that manage the request

	//	If IP is deny.		code 403
	//	If method deny.		code 405
	//	If return.			return url			(code and url is in the configuration and the response is the same for local path or url. encode url.)
	//	If error.			return error		(check inversed if error is manage in VServer or global and if it is a file, check if exists and permissions)
	//	If directory.		return directory	(code 200. check if exists and permissions)
	//	If file.			return file			(code 200. check if exists and permissions)

	//	Also return the path for access_log, error_log and theri maxsizes (if aplicable)



	//	------------------------------
	//	INFORMACION DE LOS ENCABEZADOS
	//	------------------------------

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

#pragma endregion

#pragma region Parse Header

	#pragma region Variables

		void Protocol::parse_variables(EventInfo * event) {
			if (!event) return;

			std::string path = event->header_map["Path"];
			size_t pos = std::min(path.find_first_of('?'), path.size());

			event->header_map["$request_uri"] = event->header_map["Method"] + " " + path + " " + event->header_map["Protocol"];

			event->header_map["$uri"] = path.substr(0, pos);
			event->header_map["$document_uri"] = event->header_map["$uri"];

			if (pos != path.size()) event->header_map["$args"] = path.substr(pos + 1);
			event->header_map["$query_string"] = event->header_map["$args"];

			event->header_map["$remote_addr"] = event->client->ip;
			event->header_map["$remote_port"] = Utils::ltos(event->client->port);
			if (event->type == CLIENT) {
				event->header_map["$server_addr"] = event->socket->ip;
				event->header_map["$server_port"] = Utils::ltos(event->socket->port);
			}

			event->header_map["$server_name"] = event->header_map["Host"];									//	El nombre del dominio que lo maneja o el nombre del primer dominio por defecto
			if (event->header_map["$server_name"].empty()) event->header_map["$server_name"] = event->header_map["$server_addr"];

			event->header_map["$http_referer"] = event->header_map["Referer"];
			event->header_map["$http_cookie"] = event->header_map["Cookie"];
			event->header_map["$http_user_agent"] = event->header_map["User-Agent"];
			event->header_map["$http_host"] = event->header_map["Host"];

			event->header_map["$host"] = event->header_map["Host"];
			if (event->header_map["$host"].empty()) event->header_map["$host"] = event->header_map["$server_name"];
			if (event->header_map["$host"].empty()) event->header_map["$host"] = event->header_map["$server_addr"];
		}


		//										EJEMPLO DE UNA SOLICITUD DE UN CLIENTE

		//			GET /products/details?item=123&color=red HTTP/1.1
		//			Host: www.example.com
		//			User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.121 Safari/537.36
		//			Referer: https://www.google.com/search?q=webserv
		//			Cookie: sessionid=abcdef1234567890; theme=dark

		//	$request				La solicitud completa.				               															GET/products/details?item=123&color=red HTTP/1.1
		//	$request_method			El método HTTP utilizado en la solicitud (GET, POST, PUT, DELETE, etc.)	  			        	    		GET
		//	$request_uri			Es la URI completa incluyendo la cadena de consulta (query string)		  									/products/details?item=123&color=red
		//	$uri, $document_uri		Es la URI sin incluir la cadena de consulta (query string)				 	  								/products/details
		//	$args, $query_string	Es la cadena de consulta (query string), que contiene los parámetros enviados después de ?					item=123&color=red
		//	$host					El nombre del host solicitado. Si no se especifica, se usa server_name o la dirección IP del servidor		www.example.com
		//	$remote_addr			La dirección IP del cliente que hizo la solicitud					             							203.0.113.45
		//	$remote_port			El puerto del cliente que hizo la solicitud							        								54321 
		//	$server_addr			La dirección IP del servidor que está manejando la solicitud				             					192.168.1.10
		//	$server_port			El puerto del servidor que está manejando la solicitud					 	               					80
		//	$server_name			El nombre del servidor virtual que está manejando la solicitud				        						www.example.com
		//	$http_referer			El valor de referer, que indica la página anterior a la que se hizo la solicitud        					https://www.google.com/search?q=webserv
		//	$http_cookie			El valor de la cookie enviada en la solicitud HTTP															sessionid=abcdef1234567890; theme=dark
		//	$http_host				El valor del encabezado host, que es el nombre del dominio o la dirección IP solicitada		       		  	www.example.com
		//	$http_user_agent		El contenido del encabezado user-agent, que identifica el navegador del cliente  							Mozilla/5.0 (Windows NT 10.0; Win64; x64)...

	#pragma endregion

	#pragma region Header

		int Protocol::parse_header(EventInfo * event) {
			if (!event) return (2);

			std::string header = std::string(event->read_buffer.begin(), event->read_buffer.end());				//	Create a string with the data read
			size_t pos = header.find("\r\n\r\n");																//	Find the end of the header

			if (pos == std::string::npos)	return (1);															//	Incomplete header
			else							event->header = header.substr(0, pos);								//	Get only the header content

			std::istringstream stream(header); std::string line;												//	Create some variables

			if (std::getline(stream, line)) {																	//	Read the first line
				std::istringstream first_line(line);
				std::string method, path, protocol;

				if (first_line >> method >> path >> protocol) {													//	Get the data from the first line (Method, Path and Protocol)
					event->header_map["Method"] = method;
					event->header_map["Path"] = path;
					event->header_map["Protocol"] = protocol;
				} else return (2);																				//	There are errors in the first line
			}

			while (std::getline(stream, line) && line != "\r") {												//	Read the lines (ignoring '\r')
				line.erase(line.size() - 1, 1);
				pos = line.find(':');																			//	Find ':' to split Key - Value
				if (pos != std::string::npos) event->header_map[line.substr(0, pos)] = line.substr(pos + 2);	//	Add the Key - Value to 'header_map'

				if (event->type == CLIENT) parse_variables(event);												//	Create header variables
			}

		return (0);
	}

	#pragma endregion

#pragma endregion

#pragma region Process

	void Protocol::process_request(EventInfo * event) {
		if (!event) return;

		gettimeofday(&event->response_time, NULL);														//	Reset response time

		event->header_map["Cache-Control"] = "no_cache";												//	Temporal

		event->body_maxsize = 10 * 1024 * 1024;
		parse_request(event);
		process_response(event);
	}

#pragma endregion
