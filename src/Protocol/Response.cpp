/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 11:59:50 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/23 15:45:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Protocol.hpp"
#include "Communication.hpp"

#pragma region Responses

	#pragma region Error

		void Protocol::response_error(EventInfo * event) {
			if (!event) return;

			std::string code = event->response_map["code"];
			std::string description = event->response_map["code_description"];

			std::string body =
				"<html>\n"
				"<head>\n"
				"    <title>" + code + " (" + description + ")</title>\n"
				"    <style>\n"
				"        body { text-align: center; font-family: Arial, sans-serif; }\n"
				"        h1 { font-size: 100px; margin: 20px; }\n"
				"        h2 { font-size: 50px; margin: 10px; }\n"
				"    </style>\n"
				"</head>\n"
				"<body>\n"
				"    <h1>" + code + "</h1>\n"
				"    <h2>" + description + "</h2>\n"
				"</body>\n"
				"</html>";

			std::string header =
				event->response_map["Protocol"] + " " + code + " " + description + "\r\n"
				"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
				"X-Content-Type-Options: nosniff\r\n"
				"Content-Length: " + Utils::ltos(body.size()) + "\r\n"
				"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

			event->write_info = 0;																	//	Set some flags
			event->write_size = 0;																	//	Set some flags
			event->write_maxsize = 0;																//	Set some flags
			event->response_map["header"] = header;
			event->write_buffer.clear();															//	Clear write_buffer
			event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());	//	Copy the header to write_buffer

			if (event->response_map["method"] != "HEAD") {
				event->response_map["body"] = body;
				event->response_size = body.size();
				event->write_maxsize = header.size() + body.size();										//	Set the total size of the data to be sent
				event->write_buffer.insert(event->write_buffer.end(), body.begin(), body.end());		//	Copy the body to write_buffer
			}

			if (Epoll::set(event->fd, true, true) == -1) event->client->remove();					//	Set EPOLL to monitor write events
		}

	#pragma endregion

	#pragma region Redirect

		void Protocol::response_redirect(EventInfo * event) {
			if (!event) return;

			std::string header =
				event->response_map["Protocol"] + " " + event->response_map["code"] + " " + event->response_map["code_description"] + "\r\n"
				"Location: " + event->response_map["path"] + "\r\n"
				"X-Content-Type-Options: nosniff\r\n"
				"Content-Length: 0\r\n"
				"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

			event->response_map["header"] = header;
			event->write_info = 0;																	//	Set some flags
			event->write_size = 0;																	//	Set some flags
			event->response_size = 0;
			event->write_maxsize = header.size();													//	Set the total size of the data to be sent
			event->write_buffer.clear();															//	Clear write_buffer
			event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());	//	Copy the header to write_buffer
			if (Epoll::set(event->fd, true, true) == -1) event->client->remove();					//	Set EPOLL to monitor write events
		}

	#pragma endregion

	#pragma region Directory

		bool is_directory(const std::string & path) {
			struct stat statbuf;

			if (stat(path.c_str(), &statbuf) != 0) return (false);
			return (S_ISDIR(statbuf.st_mode));
		}

		void Protocol::response_directory(EventInfo * event) {
			std::vector<std::string> files;
			std::vector<std::string> directories;
			
			std::string dir_path = event->response_map["path"];

			DIR *dir = opendir(dir_path.c_str());											// Abrir el directorio
			if (!dir) return; // return error?
			
			struct dirent *entry;
			while ((entry = readdir(dir)) != NULL) {										// Leer los contenidos del directorio
				std::string name = entry->d_name;

				if (name == "." || name == "..") continue;									// Ignorar . y ..

				std::string full_path = dir_path + "/" + name;
				if (is_directory(full_path))	directories.push_back(name + "/");
				else							files.push_back(name);
			}

			closedir(dir);

			std::string body =
				"<!DOCTYPE html>\n"
				"<html lang=\"en\">\n<head>\n"
				"<meta charset=\"UTF-8\">\n"
				"<title>Index of " + dir_path + "</title>\n"
				"</head>\n<body>\n"
				"<h1>Index of " + dir_path + "</h1>\n"
				"<ul>\n";

			if (dir_path != "/") body += "<li><a href=\"../\">Parent Directory</a></li>\n";						// Enlace al directorio padre si no estamos en la raíz

			for (std::vector<std::string>::iterator it = directories.begin(); it != directories.end(); ++it)	// Añadir directorios
				body += "<li><a href=\"" + *it + "\">" + *it + "</a></li>\n";

			for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)				// Añadir archivos
				body += "<li><a href=\"" + *it + "\">" + *it + "</a></li>\n";

			body += "</ul>\n</body>\n</html>\n";

			std::string header =
				event->response_map["Protocol"] + " " + event->response_map["code"] + " " + event->response_map["code_description"] + "\r\n"
				"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
				"X-Content-Type-Options: nosniff\r\n"
				"Content-Length: " + Utils::ltos(body.size()) + "\r\n"
				"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

			event->response_map["header"] = header;
			event->response_map["body"] = body;
			event->write_info = 0;																	//	Set some flags
			event->write_size = 0;																	//	Set some flags
			event->write_maxsize = 0;																	//	Set some flags
			event->write_buffer.clear();															//	Clear write_buffer
			event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());	//	Copy the header to write_buffer
	
			if (event->response_map["method"] != "HEAD") {
				event->response_size = body.size();
				event->write_maxsize = header.size() + body.size();											//	Set the total size of the data to be sent
				event->write_buffer.insert(event->write_buffer.end(), body.begin(), body.end());		//	Copy the body to write_buffer
			}

			if (Epoll::set(event->fd, true, true) == -1) event->client->remove();					//	Set EPOLL to monitor write events

		}

	#pragma endregion

	#pragma region File

		void Protocol::response_file(EventInfo * event) {
			if (!event) return;

			std::string path = event->response_map["path"];
			if (path.empty()) { event->client->remove(); return; }

			if (event->header_map["Cache-Control"].empty()) {											//	If cache is allowed
				CacheInfo * fcache = Communication::cache.get(path);									//	Get the file from cache
				if (fcache) {																			//	If the file exist in cache
					std::string header =
						event->response_map["Protocol"] + " " + event->response_map["code"] + " " + event->response_map["code_description"] + "\r\n"
						"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
						"X-Content-Type-Options: nosniff\r\n"
						"Content-Length: " + Utils::ltos(fcache->content.size()) + "\r\n"
						"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

					event->write_info = 0;																					//	Set some flags
					event->write_size = 0;																					//	Set some flags
					event->write_maxsize = 0;																				//	Set some flags
					event->write_buffer.clear();																			//	Clear write_buffer
					event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());					//	Copy the header to write_buffer

					if (event->response_map["method"] != "HEAD") {
						event->response_size = fcache->content.size();
						event->write_maxsize = header.size() + fcache->content.size();											//	Set the total size of the data to be sent
						event->write_buffer.insert(event->write_buffer.end(), fcache->content.begin(), fcache->content.end());	//	Copy the body to write_buffer
					}

					if (Epoll::set(event->fd, true, true) == -1) event->client->remove();									//	Set EPOLL to monitor write events
					return;
				}
			}

			int fd = open(path.c_str(), O_RDONLY);														//	Open the file
			if (fd == -1) return;																		//	If error opening, return
			Utils::NonBlocking_FD(fd);																	//	Set the FD as non-blocking

			size_t filesize = Utils::filesize(fd);														//	Get the file size
			if (filesize == std::string::npos) { close(fd); event->client->remove(); return; }


			if (event->response_map["method"] == "HEAD") {
				close(fd);
				std::string header =
					event->response_map["Protocol"] + " " + event->response_map["code"] + " " + event->response_map["code_description"] + "\r\n"
					"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
					"X-Content-Type-Options: nosniff\r\n"
					"Content-Length: " + Utils::ltos(filesize) + "\r\n"
					"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

				event->write_info = 0;																		//	Set some flags
				event->write_size = 0;																		//	Set some flags
				event->write_maxsize = 0;																	//	Set some flags
				event->write_buffer.clear();																//	Clear write_buffer
				event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());		//	Copy the header to write_buffer

				if (Epoll::set(event->fd, true, true) == -1) event->client->remove();						//	Set EPOLL to monitor write events
				return;
			}


			EventInfo event_data(fd, DATA, NULL, event->client);										//	Create the event for the DATA

			event_data.read_path = path;																//	Set the name of the file
			event_data.read_maxsize = filesize;															//	Set the size of the file
			event_data.no_cache = !event->header_map["Cache-Control"].empty();							//	Set if the file must be added to cache

			if (pipe(event_data.pipe) == -1) { close(event_data.fd); return; event->client->remove(); }	//	Create the pipe for DATA (to be used with splice)
			Utils::NonBlocking_FD(event_data.pipe[0]);													//	Set the read end of the pipe as non-blocking
			Utils::NonBlocking_FD(event_data.pipe[1]);													//	Set the write end of the pipe as non-blocking

			size_t chunk = Communication::CHUNK_SIZE;																
			if (event_data.read_maxsize > 0) chunk = std::min(event_data.read_maxsize, Communication::CHUNK_SIZE);	//	Set the size of the chunk
			if (splice(event_data.fd, NULL, event_data.pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) {				//	Send the first chunk of data from the file to the pipe
				close(event_data.fd); close(event_data.pipe[0]); close(event_data.pipe[1]);							//	Splice failed, close FD's and return
				event->client->remove(); return;
			}
			std::swap(event_data.fd, event_data.pipe[0]);												//	Swap the read end of the pipe with the fd (this is to be consistent with the FD that EPOLL is monitoring)

			Event::events[event_data.fd] = event_data;													//	Add the DATA event to the event's list

			std::string header =
				event->response_map["Protocol"] + " " + event->response_map["code"] + " " + event->response_map["code_description"] + "\r\n"
				"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
				"X-Content-Type-Options: nosniff\r\n"
				"Content-Length: " + Utils::ltos(event_data.read_maxsize) + "\r\n"
				"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

			event->read_info = 0;																		//	Set some flags
			event->read_size = 0;																		//	Set some flags
			event->read_maxsize = header.size() + event_data.read_maxsize;								//	Set the total size of the data to be sent
			event->response_size = event_data.read_maxsize;
			event->write_buffer.clear();																//	Clear write_buffer
			event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());		//	Copy the header to write_buffer

			if (Epoll::set(event->fd, !(event->header_map["Write_Only"] == "true"), true) == -1)		//	Set EPOLL to monitor write events for the client
				event->client->remove();
			else if (Epoll::add(event_data.fd, true, false) == -1) {									//	Set EPOLL to monitor read events for DATA
				event->read_maxsize = 0;																	//	If set EPOLL fails, reset the flag,
				event->write_buffer.clear();															//	clear writte_buffer
				Event::remove(event_data.fd);															//	and remove the DATA event
				event->client->remove();
			}

			return;
		}

	#pragma endregion

	#pragma region CGI

		#pragma region Variables

			void Protocol::variables_cgi(EventInfo * event, std::vector<std::string> & cgi_vars) {
				if (!event) return;

				cgi_vars.push_back("REQUEST_METHOD=" + event->header_map["Method"]);
				cgi_vars.push_back("REQUEST_URI=" + event->header_map["$request_uri"]);
				cgi_vars.push_back("QUERY_STRING=" + event->header_map["$query_string"]);
				cgi_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");

				std::string path = event->header_map["$uri"];
				size_t dot_pos = path.find('.');
				size_t slash_pos = path.find('/', dot_pos);

				if (dot_pos != std::string::npos && slash_pos != std::string::npos) {
					cgi_vars.push_back("SCRIPT_NAME=" + path.substr(0, slash_pos));
					cgi_vars.push_back("PATH_INFO=" + path.substr(slash_pos));
				} else {
					cgi_vars.push_back("SCRIPT_NAME=" + path);
					cgi_vars.push_back("PATH_INFO=");
				}
				cgi_vars.push_back("PATH_TRANSLATED=" + event->response_map["path"]);

				cgi_vars.push_back("CONTENT_TYPE=" + event->header_map["Content-Type"]);
				cgi_vars.push_back("CONTENT_LENGTH=" + event->header_map["Content-Length"]);

				cgi_vars.push_back("SERVER_NAME=" + event->header_map["$host"]);
				cgi_vars.push_back("SERVER_PORT=" + event->header_map["$server_port"]);
				cgi_vars.push_back("SERVER_PROTOCOL=" + event->header_map["Protocol"]);
				cgi_vars.push_back("REMOTE_ADDR=" + event->header_map["$remote_addr"]);
				cgi_vars.push_back("REMOTE_PORT=" + event->header_map["$remote_port"]);

				cgi_vars.push_back("HTTP_HOST=" + event->header_map["$server_name"]);
				cgi_vars.push_back("HTTP_USER_AGENT=" + event->header_map["User-Agent"]);
				cgi_vars.push_back("HTTP_REFERER=" + event->header_map["Referer"]);
				cgi_vars.push_back("HTTP_ACCEPT=" + (event->header_map["Accept"].empty() ? "*/*" : event->header_map["Accept"]));
			}
		
			#pragma region Information

				//										EJEMPLO DE UNA SOLICITUD DE UN CLIENTE

				//			POST /cgi-bin/script.cgi/extra/path?item=123&color=red HTTP/1.1
				//			Host: www.example.com
				//			User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.121 Safari/537.36
				//			Referer: https://www.google.com/search?q=webserv
				//			Cookie: sessionid=abcdef1234567890; theme=dark
				//			Accept: text/html,application/xhtml+xml
				//			Content-Type: text/html
				//			Content-Length: 1234

				//	REQUEST_METHOD		El método HTTP utilizado en la solicitud (GET, POST, PUT, DELETE, etc.)		  			            		POST
				//	REQUEST_URI			Es la URI completa incluyendo la cadena de consulta (query string)		  									/products/details?item=123&color=red
				//	QUERY_STRING		Es la cadena de consulta (query string), que contiene los parámetros enviados después de ?					item=123&color=red

				//	GATEWAY_INTERFACE	La versión de CGI que está usando el servidor web															CGI/1.1
				//	SCRIPT_NAME			La ruta virtual del script CGI que está siendo ejecutado, tal como se solicitó en la URL					/cgi-bin/script.cgi
				//	PATH_INFO			Cualquier información adicional después del nombre del script en la URL										/extra/path
				//	PATH_TRANSLATED		La ruta completa en el sistema de archivos del recurso solicitado											/var/www/cgi-bin/extra/path

				//	CONTENT_TYPE		Indica el tipo de contenido de los datos. Puede estar vacío													text/html
				//	CONTENT_LENGTH		Indica la longitud del cuerpo de la solicitud en bytes. Puede estar vacío									1234

				//	SERVER_NAME			El valor del encabezado host, que es el nombre del dominio o la dirección IP solicitada		     	    	www.example.com
				//	SERVER_PORT			El puerto del servidor que está manejando la solicitud					 	               					80
				//	SERVER_PROTOCOL		El protocolo HTTP que está utilizando el cliente															HTTP/1.1
				//	REMOTE_ADDR			La dirección IP del cliente que hizo la solicitud					             							203.0.113.45
				//	REMOTE_PORT			El puerto del cliente que hizo la solicitud							        								54321 

				//	HTTP_HOST			El nombre del servidor virtual que está manejando la solicitud				        						www.example.com
				//	HTTP_ACCEPT			Los tipos de contenido que el cliente está dispuesto a aceptar												text/html,application/xhtml+xml
				//	HTTP_REFERER		El valor de referer, que indica la página anterior a la que se hizo la solicitud        					https://www.google.com/search?q=webserv
				//	HTTP_USER_AGENT		El contenido del encabezado user-agent, que identifica el navegador del cliente  							Mozilla/5.0 (Windows NT 10.0; Win64; x64)...

				//	El cuerpo de la solicitud se envia al CGI a través del STDIN

				// Ejemplo: execve("/usr/bin/python", ["python", "/path/to/script.cgi"], envp);

			#pragma endregion

		#pragma endregion

		#pragma region Fork

			void Protocol::response_cgi(EventInfo * event) {
				if (!event) return;

			//	Create the event to write to the CGI
				if (event->header_map["Content-Length"].empty() == false) {
					int write_pipe[2];
					if (pipe(write_pipe) == -1) { event->client->remove(); }								//	Create the pipe for CGI (read from it)
					Utils::NonBlocking_FD(write_pipe[0]);													//	Set the read end of the pipe as non-blocking
					Utils::NonBlocking_FD(write_pipe[1]);													//	Set the write end of the pipe as non-blocking

					EventInfo event_write_cgi(write_pipe[1], DATA, NULL, event->client);					//	Create the event for the CGI
					event_write_cgi.pipe[0] = write_pipe[0];
					event_write_cgi.pipe[1] = -1;
					event->cgi_fd = event_write_cgi.fd;
					Event::events[event_write_cgi.fd] = event_write_cgi;									//	Add the CGI event to the event's list
				}

			//	Create the event to read from the CGI
				int read_pipe[2];
				if (pipe(read_pipe) == -1) { event->client->remove(); }										//	Create the pipe for CGI (read from it)
				Utils::NonBlocking_FD(read_pipe[0]);														//	Set the read end of the pipe as non-blocking
				Utils::NonBlocking_FD(read_pipe[1]);														//	Set the write end of the pipe as non-blocking

				EventInfo event_read_cgi(read_pipe[0], DATA, NULL, event->client);							//	Create the event for the CGI
				event_read_cgi.pipe[0] = -1;
				event_read_cgi.pipe[1] = read_pipe[1];

				Event::events[event_read_cgi.fd] = event_read_cgi;											//	Add the CGI event to the event's list

			//	Set EPOLL
				event->write_buffer.clear();																//	Clear write_buffer

				if (Epoll::set(event->fd, !(event->header_map["Write_Only"] == "true"), true) == -1) {		//	Set EPOLL to monitor write events for the client
					event->client->remove(); return;
				} else {
					if (event->cgi_fd != -1 && Epoll::add(event->cgi_fd, false, true) == -1) {				//	Set EPOLL to monitor write events for CGI
						event->write_maxsize = 0;															//	If set EPOLL fails, reset the flag,
						event->client->remove(); return;
					}
					if (Epoll::add(event_read_cgi.fd, true, false) == -1) {									//	Set EPOLL to monitor read events for CGI
						event->read_maxsize = 0;															//	If set EPOLL fails, reset the flag,
						event->client->remove(); return;
					}
				}

			//	Fork the process
				int pid = fork();
				if (pid == -1) {
			
				
				} else if (pid == 0) {
					Epoll::close();
					for (std::map <int, EventInfo>::iterator it = Event::events.begin(); it != Event::events.end(); ++it)
						if (it->first) close(it->first);

					std::vector<char *> env_array;
					std::vector<std::string> cgi_vars;
					variables_cgi(event, cgi_vars);

					char * args[3];
					args[0] = const_cast<char *>(event->response_map["cgi_path"].c_str());
					args[1] = const_cast<char *>(event->response_map["path"].c_str());
					args[2] = NULL;
					
					for (size_t i = 0; i < cgi_vars.size(); ++i)
						env_array.push_back(const_cast<char*>(cgi_vars[i].c_str()));
					env_array.push_back(NULL);

					if (event->cgi_fd != -1) dup2(event->cgi_fd, STDIN_FILENO);
					dup2(event_read_cgi.fd, STDOUT_FILENO);

					if (execve(args[0], args, &env_array[0]) == -1) exit(1);
				}
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Process

	void Protocol::process_response(EventInfo * event) {
		if (!event) return;

		if 		(event->method == "Error")		response_error(event);
		else if (event->method == "Redirect")	response_redirect(event);
		else if (event->method == "Directory")	response_directory(event);
		else if (event->method == "File")		response_file(event);
		else if (event->method == "CGI")		response_cgi(event);
		else {
			//	Set a diferent error
			response_error(event);
		}
	}

#pragma endregion
