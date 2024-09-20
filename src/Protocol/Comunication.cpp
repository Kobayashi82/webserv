/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Comunication.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 09:32:08 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/20 14:23:43 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Comunication.hpp"
#include "Display.hpp"
#include "Thread.hpp"
#include "Client.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"

//	!	IMPORTANT	The response must obtain the file size, generate the header and then read the file and write to the client at the same time so we dont have to load in memory a big file.
//	!				This is in case the file is too big, if not, we can cache the file.
//	!				The problem is if the file asked is too big and with the current implementation it will be loaded in memory. A file of 1 GB is not aceptable.

//	!	IMPORTANT	In read_client i have to check for a new header in the buffer and then process the request. After that continue reading. (really?)
//	!				So, i need a function to check if a header is in the buffer and where start a header (ignoring the first header, of course)

//	TODO	uploads es necesario?
//	TODO	Verifica el tipo MIME del archivo y el nombre del archivo para asegurarte de que coincide con el tipo esperado.
//	TODO	Limita el tamaño máximo de las solicitudes HTTP. Esto puede prevenir que un atacante agote los recursos del servidor con solicitudes grandes.


//	*	CGI STEPS

// Al leer de un archivo o cgi, si no se guarda en cache.

// Si es un archivo:

// Leo un chunk y lo envío al cliente
// El cliente sabe cuantos datos son
// Si no hay mas datos o he leído el total del archivo, cierro

// Si es un cgi

// Leo el primer chunk y saco el tamaño de content_lenth

// Si hay tamaño

// Leo un chunk y lo envío al cliente
// El cliente sabe cuantos datos son
// Si he leido el total de content_length cierro
// Si no y tarda mucho, timeout

// Si no hay tamaño 

// Event->manual_stop = 1;
// El cliente no sabe cuantos datos son
// Leo un chunk y lo envío al cliente 
// Cuando llegue a un eof o timeout
// Event->manual_stop = 2
// El cliente sabe que ha terminado y cierra

#pragma region Variables

	std::list<Client>	Comunication::clients;															//	List of all Client objects
	Cache				Comunication::cache(600, 100, 10);												//	Used to store cached data, such as files or HTML responses.	(arguments: expiration in seconds, max entries, max content size in MB)

	int					Comunication::total_clients;													//	Total number of clients conected
	size_t				Comunication::read_bytes;														//	Total number of bytes downloaded by the server
	size_t				Comunication::write_bytes;														//	Total number of bytes uploaded by the server

	const size_t		Comunication::CHUNK_SIZE = 4096;												//	Size of the buffer for read and write operations

#pragma endregion

int parse_header(EventInfo * event) {
	std::string header = std::string(event->read_buffer.begin(), event->read_buffer.end());
	size_t pos = header.find("\r\n\r\n");

    if (pos == std::string::npos)	return (1);													// Encabezado incompleto, devuelve el mapa vacío
	else							event->header = header.substr(0, pos);

    std::istringstream stream(header);
    std::string line;

    // Lee la primera línea (Request line: Method Path HTTP-Version)
    if (std::getline(stream, line)) {
        std::istringstream request_line(line);
        std::string method, path, http_version;

        if (request_line >> method >> path >> http_version) {
            event->header_map["Method"] = method;
            event->header_map["Path"] = path;
            event->header_map["Protocol"] = http_version;
        } else return (2);
    }

    // Lee el resto de las líneas (encabezados clave: valor)
    while (std::getline(stream, line) && line != "\r") {
        pos = line.find(':');
        if (pos != std::string::npos) event->header_map[line.substr(0, pos)] = line.substr(pos + 2);
    }

    return (0);
}

#pragma region Comunications

	#pragma region CLIENT

		#pragma region Read

			int Comunication::read_client(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));
				char peek_buffer[CHUNK_SIZE + 1];	memset(peek_buffer, 0, sizeof(peek_buffer));

				ssize_t bytes_peek = recv(event->fd, peek_buffer, CHUNK_SIZE + 1, MSG_PEEK);
				ssize_t bytes_read = recv(event->fd, buffer, CHUNK_SIZE, 0);

				if (bytes_read > 0) {

					event->client->update_last_activity();
					Event::update_last_activity(event->fd);

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);

					Thread::inc_size_t(Display::mutex, read_bytes, bytes_read);

					if (event->header == "") {
						int result = parse_header(event);
						if (result == 0) {
							process_request(event);
						} else if (result == 2) {
							event->client->remove(); return (1);
						}
					}

					if (event->file_info == CGI) {
						event->read_buffer.clear();
						EventInfo * cgi_event = Event::get(event->cgi_fd);
						if (!cgi_event) event->client->remove(); return (1);
						cgi_event->write_buffer.insert(cgi_event->write_buffer.end(), buffer, buffer + bytes_read);
					}

					if (static_cast<size_t>(bytes_peek) <= CHUNK_SIZE) return (1);

				} else if (bytes_read == 0) {															//	No more data
					event->client->remove(); return (1);
				} else if (bytes_read == -1) {															//	Error reading
					Log::log(RED500 "Comunication failed with " BLUE400 + event->client->ip + NC, Log::BOTH_ERROR, event->socket->VServ);
					event->client->remove(); return (1);
				}		

				return (0);
			}

		#pragma endregion

		#pragma region Write

			void Comunication::write_client(EventInfo * event) {
				if (!event) return;

				if (!event->write_buffer.empty()) {

					event->client->update_last_activity();
					Event::update_last_activity(event->fd);

					size_t buffer_size = event->write_buffer.size();
					size_t chunk = CHUNK_SIZE;
					if (buffer_size > 0) chunk = std::min(buffer_size, static_cast<size_t>(CHUNK_SIZE));
					
					ssize_t bytes_written = write(event->fd, event->write_buffer.data(), chunk);

					if (bytes_written > 0) {
						event->write_buffer.erase(event->write_buffer.begin(), event->write_buffer.begin() + bytes_written);

						event->file_read += bytes_written;

						Thread::inc_size_t(Display::mutex, write_bytes, bytes_written);

					} else { event->client->remove(); return; }					//	Error writing
				}

				if ((event->file_info == 0 && event->file_read >= event->file_size) || (event->file_info == 2 && event->write_buffer.empty())) {
					long MaxRequests = Settings::KEEP_ALIVE_TIMEOUT;
					if (Settings::global.get("keepalive_requests") != "") Utils::stol(Settings::global.get("keepalive_requests"), MaxRequests);

					Log::log("GET", "/", 200, write_bytes, "250", event->client->ip, event->socket->VServ, event->vserver_data);

					if (event->header_map["Connection"] == "close" || event->client->total_requests + 1 >= MaxRequests)
						event->client->remove();
					else {
						Epoll::set(event->fd, true, false);
						event->client->total_requests++;
					}
				}

			}

		#pragma endregion

	#pragma endregion

	#pragma region DATA

		#pragma region Read

			int Comunication::read_data(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));

				ssize_t bytes_read = read(event->fd, buffer, CHUNK_SIZE);

				if (bytes_read > 0) {

					Event::update_last_activity(event->fd);

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);
					event->file_read += bytes_read;

					if (event->no_cache == true) {
						EventInfo * c_event = Event::get(event->client->fd);
						c_event->write_buffer.insert(c_event->write_buffer.end(), buffer, buffer + bytes_read);
						if (event->file_read >= event->file_size) {
							c_event->file_info = 2;
							Event::remove(event->fd); return (1);
						}

					} else if (event->file_read >= event->file_size) {
						std::string data(event->read_buffer.begin(), event->read_buffer.end());
						cache.add(event->file_path, data);

						EventInfo * c_event = Event::get(event->client->fd);
						c_event->file_info = 2;
						c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());
						Event::remove(event->fd); return (1);
					}

				} else if (bytes_read == 0) {														//	No more data
					EventInfo * c_event = Event::get(event->client->fd);
					c_event->file_info = 2;
					if (event->no_cache == false) c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());
					Event::remove(event->fd); return (1);
				} else if (bytes_read == -1) {														//	Error reading
					EventInfo * c_event = Event::get(event->client->fd);
					c_event->file_info = 2;
					if (event->no_cache == false) c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());
					Event::remove(event->fd); return (1);
				}

				size_t chunk = CHUNK_SIZE;
				if (event->file_size > 0) chunk = std::min(event->file_size - event->file_read, CHUNK_SIZE);
				if (splice(event->pipe[0], NULL, event->pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) { Event::remove(event->fd); return (1); }

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region CGI

		#pragma region Read

			int Comunication::read_cgi(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));

				ssize_t bytes_read = read(event->fd, buffer, CHUNK_SIZE);

				if (bytes_read > 0) {

					Event::update_last_activity(event->fd);

					EventInfo * c_event = Event::get(event->client->fd);

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);
					event->file_read += bytes_read;

					if (event->file_info == 0 && event->file_size == 0) {								//	Needs to get 'content_length'
						if (event->header == "") {
							size_t content_length = 0;
							int result = parse_header(event);
							if (result == 0) {
								content_length = Utils::stol(event->header_map["Content_length"], content_length);
								if (content_length == 0) event->file_info = 1;							//	No 'content_length'
								else {
									c_event->file_info = event->file_info;
									c_event->file_size = event->file_size;
								}
							} else if (result == 2) {
								Event::remove(event->fd); return (1);
							}
						}
					}

					c_event->write_buffer.insert(c_event->write_buffer.end(), buffer, buffer + bytes_read);

					if (event->file_info == 0 && event->file_read >= event->file_size) {				//	Read all data 'content_length'
						c_event->file_info = 2;
						Event::remove(event->fd); return (1);
					}

				} else if (bytes_read == 0 && event->file_info == 1) {									//	No more data
					Event::get(event->client->fd)->file_info = 2;
					Event::remove(event->fd); return (1);
				} else if (bytes_read == -1) {															//	Error reading
					Event::get(event->client->fd)->file_info = 2;
					Event::remove(event->fd); return (1);
				}

				return (0);
			}

		#pragma endregion

		#pragma region Write

			void Comunication::write_cgi(EventInfo * event) {
				if (!event) return;

				if (!event->write_buffer.empty()) {

					Event::update_last_activity(event->fd);

					size_t buffer_size = event->write_buffer.size();
					size_t chunk = CHUNK_SIZE;
					if (buffer_size > 0) chunk = std::min(buffer_size, static_cast<size_t>(CHUNK_SIZE));
					
					ssize_t bytes_written = write(event->fd, event->write_buffer.data(), chunk);

					if (bytes_written > 0) {
						event->write_buffer.erase(event->write_buffer.begin(), event->write_buffer.begin() + bytes_written);

						event->file_read += bytes_written;

					} else { Event::remove(event->fd); return; }										//	Error writing
				}

				if ((event->file_info == 0 && event->file_read >= event->file_size) || (event->file_info == 2 && event->write_buffer.empty())) {
					Event::remove(event->fd);
				}
			}

		#pragma endregion

	#pragma endregion

	#pragma region PROCESS

		#pragma region Request

			//	This temporary function process the request

			void Comunication::process_request(EventInfo * event) {
				if (!event) return;

				event->close = true;																	//	For now lets close the connection always
				event->no_cache = true;
				//	if (body > max_body_size) return lo que sea

				// std::istringstream request_stream(request); std::string line;
				// if (std::getline(request_stream, line)) Utils::trim(line);
				// Log::log(line, Log::BOTH_ACCESS);
				//Log::log("Prueba", Log::BOTH_ACCESS);
				process_response(event);																//	ELIMINAR
			}

		#pragma endregion

		#pragma region Response

			//	This temporary function process the response
			//	It could be refactorized to use it as a function for serving a file

			void Comunication::process_response(EventInfo * event) {
				if (!event) return;

				if (event->header_map["Cache-Control"].empty()) {
					CacheInfo * fcache = cache.get("index.html");
					if (fcache) {
						std::string response = 
							"HTTP/1.1 200 OK\r\n"
							"Content-Type: text/html\r\n"
							"Content-Length: " + Utils::ltos(fcache->content.size()) + "\r\n"
							"Connection: close\r\n"
							"\r\n";

						event->file_info = 0;
						event->file_read = 0;
						event->file_size = response.size() + fcache->content.size();
						event->write_buffer.clear();
						event->write_buffer.insert(event->write_buffer.end(), response.begin(), response.end());
						event->write_buffer.insert(event->write_buffer.end(), fcache->content.begin(), fcache->content.end());
						if (Epoll::set(event->fd, true, true) == -1) {
							event->file_size = 0;
							event->write_buffer.clear();
						}

						return;
					}
				}

				int fd = open("index.html", O_RDONLY);
				if (fd == -1) return;
				Utils::NonBlocking_FD(fd);

				size_t filesize = Utils::filesize(fd);
				if (filesize == std::string::npos) { close(fd); return; }

				EventInfo event_data(fd, DATA, NULL, event->client);

				event_data.file_path = "index.html";
				event_data.file_size = filesize;
				event_data.no_cache = !event->header_map["Cache-Control"].empty();

				if (pipe(event_data.pipe) == -1) { close(event_data.fd); return; }						//	Create pipe
				Utils::NonBlocking_FD(event_data.pipe[0]);
				Utils::NonBlocking_FD(event_data.pipe[1]);

				size_t chunk = CHUNK_SIZE;
				if (event_data.file_size > 0) chunk = std::min(event_data.file_size, CHUNK_SIZE);
				if (splice(event_data.fd, NULL, event_data.pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) {
					close(event_data.fd); close(event_data.pipe[0]); close(event_data.pipe[1]); return;
				}
				std::swap(event_data.fd, event_data.pipe[0]);

				Event::events[event_data.fd] = event_data;

				std::string response = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"Content-Length: " + Utils::ltos(event_data.file_size) + "\r\n"
					"Connection: close\r\n"
					"\r\n";

				event->file_info = 0;
				event->file_read = 0;
				event->file_size = response.size() + event_data.file_size;
				event->write_buffer.clear();
				event->write_buffer.insert(event->write_buffer.end(), response.begin(), response.end());
				if (Epoll::set(event->fd, true, true) == -1 || Epoll::add(event_data.fd, true, false) == -1) {
					event->file_size = 0;
					event->write_buffer.clear();
					Event::remove(event_data.fd); return;
				}
				return;
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Resolve Request																			//	Not valid really... is old

	//	Return structure with info about the result
	void Comunication::resolve_request(const std::string host, const std::string method, std::string path, EventInfo * event) {
		(void) host;
		(void) method;
		(void) path;
		(void) event;
		//	host	(host name of the connection)
		//	method	(GET, POST, DELETE, ERROR)
		//	path	(split between path and query)
		//	event	(event of the client)

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

	}

	//	void Comunication::process_request(EventInfo * event, std::string request)
	//
	//	Esta es la funcion de entrada para el procesado de lo que pide el cliente
	//	event es un puntero al EventInfo del cliente. 
	//	El EventInfo se guarda en Comunication::events y es un map de FD - EventInfo
	//	Para ver la implentacion de EventInfo, está en Net.hpp
	//
	//	request es un string con el contenido de la peticion.
	//	Esto incluye el header y body.
	//	Tu me pasarias a la funcion intermediaria el metodo (GET, POST, etc...), la ruta (/folder por ejemplo) y
	//	el * event (para obtener la ip del cliente).
	//
	//	Eso te devolverá una estructura que todavia no esta hecha, pero que te dará la información necesaria para
	//	construir la respuesta.

#pragma endregion
