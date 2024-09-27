/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 11:52:00 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/28 01:25:08 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Protocol.hpp"
#include "Communication.hpp"

#pragma region Parsers

	#pragma region Request

		//	TODO	data of vserver/location in event
		//	TODO	Index usa index.html por defecto
		//	TODO	Update resource path with alias or any modified path before cgi

		//	*	Disabled modification time
		//	*	Access to disk super slow
		//	*	127.0.0.1 not receiving logs

		//	?		Internal... is necessary?

	#pragma region Error Page

		bool file_exists(const std::string& path) {
			struct stat fileStat;

			if (stat(path.c_str(), &fileStat) != 0) return (false);
			if (fileStat.st_mode & S_IRUSR) return (true);			// Verificar si tiene permisos de lectura para el usuario actual

			return (false);
		}

		int error_page(EventInfo * event, std::string code, VServer * VServ, Location * Loc = NULL) {
			std::string temp;
			std::string root;

			if (Loc) root = Loc->get("root");
			if (root.empty() && VServ) root = VServ->get("root");
			if (root.empty()) root = Settings::global.get("root");

			if (Loc) temp = Loc->get("error_page " + code);
			if (temp.empty() && VServ) temp = VServ->get("error_page " + code);
			if (temp.empty()) temp = Settings::global.get("error_page " + code);

			if (!VServ) VServ = &Settings::global;

			if (temp.empty() || !file_exists(root + "/" + temp)) {// Utils::file_exists(root + "/" + temp)) {
				event->response_map["Method"] = "Error";
				event->response_map["Code"] = code;
			} else {
				event->response_map["Method"] = "File";
				event->response_map["Code"] = code;
				event->response_map["Path-Full"] = root + "/" + temp;
				event->response_map["Path"] = root + "/" + temp;
			}

			event->VServ = VServ; event->Loc = Loc;
			if (Loc)		event->vserver_data = &Loc->data;
			else if (VServ)	event->vserver_data = &VServ->data;

			return (1);
		}

	#pragma endregion

	#pragma region Redirect

		static int redirect(EventInfo * event, std::string code_path, VServer * VServ, Location * Loc = NULL) {
			std::string code, path;

			size_t pos = code_path.find_first_of(' ');
			if (pos == std::string::npos) return (0);

			code = code_path.substr(0, pos);
			path = code_path.substr(pos + 1);

			event->response_map["Method"] = "Redirect";
			event->response_map["Code"] = code; // 301, 302, 303, 307, 308
			event->response_map["Path"] = path;

			if (!VServ) VServ = &Settings::global;

			event->VServ = VServ; event->Loc = Loc;
			if (Loc)		event->vserver_data = &Loc->data;
			else if (VServ)	event->vserver_data = &VServ->data;

			return (1);
		}

	#pragma endregion

	#pragma region Method

		int method(EventInfo * event, int index, VServer * VServ, Location * Loc = NULL) {
			Method * Met = NULL;

			if (Loc) Met = &Loc->method[index];
			if (!Met && VServ) Met = &VServ->method[index];
			if (!Met) Met = &Settings::global.method[index];
			if (!Met || Met->get("method").find(event->header_map["Method"]) == std::string::npos) return (0);

			bool allowed = false;

			std::vector<std::pair<std::string, std::string> >::const_iterator it;
			for (it = Met->data.begin(); it != Met->data.end(); ++it) {
				if (it->first == "allow" && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) allowed = true;
				if (it->first == "deny" && allowed == false && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) return (error_page(event, "403", VServ));
				if (it->first == "return") return (redirect(event, it->second, VServ, Loc));
			}

			return (0);
		}

	#pragma endregion

	#pragma region Location

		int location(EventInfo * event, int index, VServer * VServ) {
			Location * Loc = NULL;

			if (VServ) Loc = &VServ->location[index];
			if (!Loc) return (0);

			bool allowed = false;

			std::string temp, root = Settings::global.get("root");

			event->VServ = VServ; event->Loc = Loc; event->vserver_data = &Loc->data;

			std::vector<std::pair<std::string, std::string> >::const_iterator it;
			for (it = Loc->data.begin(); it != Loc->data.end(); ++it) {
				if (it->first == "body_maxsize") event->body_maxsize = Utils::sstol(it->second);
				if (it->first == "allow" && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) allowed = true;
				if (it->first == "deny" && allowed == false && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) return (error_page(event, "403", NULL));
				if (it->first == "return") return (redirect(event, it->second, NULL));
				if (it->first == "method" && method(event, Utils::sstol(it->second), NULL)) return (1);
			}

			return (0);
		}

	#pragma endregion

	#pragma region VServers

		bool isAddress(VServer * VServ, const std::string & ip, int port) {
			std::vector<std::pair<std::string, int> >::const_iterator it;
		    for (it = VServ->addresses.begin(); it != VServ->addresses.end(); ++it) {
				if (it->first == "0.0.0.0" && it->second == port) return (true);
		        if (it->first == ip && it->second == port) return (true);
			}

		    return (false);
		}

		int vservers(EventInfo * event) {
			VServer * default_VServ = NULL;
			VServer * VServ = NULL;

			std::deque<VServer>::iterator v_it;
			for (v_it = Settings::vserver.begin(); v_it != Settings::vserver.end(); ++v_it) {
				VServer * tmp_serv = &(*v_it);

				if (v_it->bad_config == true || isAddress(tmp_serv, event->socket->ip, event->socket->port) == false) continue;
				if (!default_VServ) default_VServ = tmp_serv;

				bool valid_name = false;
				std::string server_name; std::istringstream iss(Utils::strToLower(tmp_serv->get("server_name")));
				while (iss >> server_name)
					if (server_name == Utils::strToLower(event->header_map["Host"])) { VServ = tmp_serv; valid_name = true; break; }
				if (valid_name) break;
			}

			if (!VServ) VServ = default_VServ;
			if (!VServ) { event->VServ = &Settings::global; event->Loc = NULL; event->vserver_data = &Settings::global.data; return (0); }

			bool allowed = false;

			event->VServ = VServ; event->Loc = NULL; event->vserver_data = &VServ->data;

			std::vector<std::pair<std::string, std::string> >::const_iterator it;
			for (it = VServ->data.begin(); it != VServ->data.end(); ++it) {
				if (it->first == "body_maxsize") event->body_maxsize = Utils::sstol(it->second);
				if (it->first == "allow" && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) allowed = true;
				if (it->first == "deny" && allowed == false && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) return (error_page(event, "403", VServ));
				if (it->first == "return") return (redirect(event, it->second, VServ));
				if (it->first == "method" && method(event, Utils::sstol(it->second), VServ)) return (1);
			}

			//std::string temp, root = Settings::global.get("root");
			//return (error_page(event, "404", VServ));
			//bool allowed = false;

			return (0);
		}

	#pragma endregion

	#pragma region Global

		int global(EventInfo * event) {
			std::string temp, root = Settings::global.get("root");
			
			bool allowed = false;

			event->VServ = &Settings::global; event->Loc = NULL; event->vserver_data = &Settings::global.data;

			std::vector<std::pair<std::string, std::string> >::const_iterator it;
			for (it = Settings::global.data.begin(); it != Settings::global.data.end(); ++it) {
				if (it->first == "body_maxsize") event->body_maxsize = Utils::sstol(it->second);
				if (it->first == "allow" && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) allowed = true;
				if (it->first == "deny" && allowed == false && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) return (error_page(event, "403", NULL));
				if (it->first == "return") return (redirect(event, it->second, NULL));
				if (it->first == "method" && method(event, Utils::sstol(it->second), NULL)) return (1);
			}

			return (0);
		}

	#pragma endregion

		void Protocol::parse_request(EventInfo * event) {
			if (!event) return;

			std::string path = event->header_map["$uri"];
			size_t dot_pos = path.find('.');
			size_t slash_pos = path.find('/', dot_pos);

			event->response_map["Path-Req"] = path.substr(0, slash_pos);
			if (dot_pos != std::string::npos && slash_pos != std::string::npos) {
				event->response_map["Path"] = path.substr(0, slash_pos);
				event->response_map["Path-Info"] = path.substr(slash_pos);
			} else {
				event->response_map["Path"] = path;
				event->response_map["Path-Info"] = "";
			}
			event->response_map["Path-Full"] = "";

			event->response_map["Protocol"] = "HTTP/1.1";
			event->response_map["Connection"] = event->header_map["Connection"];
			if (event->response_map["Connection"].empty()) event->response_map["Connection"] = "keep-alive";

			event->response_map["Server"] = Settings::server_name + "/" + Settings::server_version + Settings::os_name;
			event->response_map["Date"] = Settings::timer.current_time_header();

			if (std::string("HEAD|GET|POST|PUT|PATCH|DELETE").find(event->header_map["Method"]) == std::string::npos) {
				error_page(event, "405", event->VServ, event->Loc);
			} else if (event->response_map["Method"] != "CGI" && event->header_map["Method"] != "HEAD" && event->header_map["Method"] != "GET") {
				error_page(event, "403", event->VServ, event->Loc);
			} else {
				if (!global(event) && !vservers(event)) {
					error_page(event, "500", event->VServ, event->Loc);
						// index and root of global /
				} else {
					event->response_map["Method"] = "File";															//	Temporal
					event->response_map["Code"] = "200";
					event->response_map["Path"] = "index.html";														//	Temporal
				}
			}

			// event->response_map["Method"] = "File";
			// event->response_map["Code"] = "200";
			// event->response_map["Path"] = "index.html";

		//	Method
			size_t content_length = 0;
			Utils::stol(event->header_map["Content-Length"], content_length);								//	Get 'content_length' in numeric format
			if (event->body_maxsize > 0 && content_length > event->body_maxsize) {							//	If 'content_length' is greater than 'body_maxsize'
				error_page(event, "403", event->VServ, event->Loc);											//	Body too big, return error
			}


		//	If (MIME != response type) { }

		event->response_map["Content-Type"] = "text/plain";

		//if (event->response_map["Method"] != "Error") event->response_map["Method"] = "File";

		//	If Method is Directory
			// if (event->response_map["Method"] == "Directory") {
			// 	event->response_map["Path"] = event->header_map["Path"];
			// }

		//	If Method is File
			if (event->response_map["Method"] == "File") {

				size_t pos1 = event->header_map["Cache-Control"].find("no-cache");
				size_t pos2 = event->header_map["Cache-Control"].find("no-store");
				event->no_cache = (pos1 != std::string::npos || pos2 != std::string::npos);

				//event->response_map["Path"] = event->header_map["Path"].substr(1);
				if (event->response_map["Path"].empty()) event->response_map["Path"] = "index.html";

				//event->response_map["Path"] = "foto.jpg";
				//event->response_map["Path"] = "big2.mp4";
				//event->response_map["Path"] = "big.mkv";

				size_t pos = event->response_map["Path"].find_last_of('.');
				if (pos != std::string::npos) event->response_map["Content-Type"] = Settings::mime_types[event->response_map["Path"].substr(pos + 1)];
			}

		//	If Method is CGI
			if (event->response_map["Method"] == "CGI") {
				event->response_map["CGI-Path"] = "cgi-bin/php-cgi";
				//event->response_map["CGI-Path"] = "cgi-bin/python-cgi";
				//event->response_map["CGI-Path"] = "/bin/cat";

				event->response_map["Path"] = "index.php";
				//event->response_map["Path"] = "index.py";
			}

			event->response_map["Code-Description"] = Settings::error_codes[Utils::sstol(event->response_map["Code"])];
			event->response_method = event->response_map["Method"];
		}

		#pragma region Information

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

	#pragma endregion

	#pragma region Code

		int Protocol::parse_code(EventInfo * event, int code) {
			(void) event;
			(void) code;

			//	Check if there are operations with the code and send the response to the cliente
			return (0);
		}

	#pragma endregion

	#pragma region Header

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

				if (event->read_buffer.empty()) return (1);

				std::string header = std::string(event->read_buffer.begin(), event->read_buffer.end());				//	Create a string with the data read
				size_t pos = header.find("\r\n\r\n");																//	Find the end of the header

				if (pos == std::string::npos)	return (1);															//	Incomplete header
				else							event->header = header.substr(0, pos);								//	Get only the header content

				std::istringstream stream(header); std::string line;

				if (std::getline(stream, line)) {																	//	Read the first line
					std::istringstream first_line(line);
					std::string value1, value2, value3;

					if (event->type == CLIENT) {
						if (first_line >> value1 >> value2 >> value3) {												//	Get the data from the first line (Method, Path and Protocol)
							event->header_map["Method"] = value1;
							event->header_map["Path"] = Security::decode_url(value2);
							event->header_map["Protocol"] = value3;
						} else return (2);																			//	There are errors in the first line
					} if (event->type == CGI) {
						if (first_line >> value1 >> value2 >> value3) {												//	Get the data from the first line (Protocol, Code and Code description)
							event->header_map["Protocol"] = value1;
							event->header_map["Code"] = value2;
							event->header_map["Code-Description"] = value3;

							if (parse_code(event, Utils::sstol(value2))) return (3);								//	The server will manage the response
						} else return (2);																			//	There are errors in the first line
					}
				}

				while (std::getline(stream, line) && line != "\r") {												//	Read the lines (ignoring '\r')
					line.erase(line.size() - 1, 1);
					pos = line.find(':');																			//	Find ':' to split Key - Value
					if (pos != std::string::npos) event->header_map[line.substr(0, pos)] = line.substr(pos + 1);	//	Add the Key - Value to 'header_map'

					if (event->type == CLIENT) parse_variables(event);												//	Create header variables
				}
				pos = event->header_map["Host"].find_first_of(':');
				if (pos != std::string::npos) event->header_map["Host"] = Utils::strim(event->header_map["Host"].substr(0, pos));

			return (0);
		}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Process

	void Protocol::process_request(EventInfo * event) {
		if (!event) return;

		gettimeofday(&event->response_time, NULL);														//	Reset response time

		parse_request(event);
		process_response(event);
	}

#pragma endregion
