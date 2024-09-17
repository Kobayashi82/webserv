/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Comunications.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 09:32:08 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/17 14:25:31 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Colors.hpp"
#include "Net.hpp"
#include "Client.hpp"
#include "Display.hpp"
#include "Thread.hpp"

//	!	IMPORTANT	The response must obtain the file size, generate the header and then read the file and write to the client at the same time so we dont have to load in memory a big file.
//	!				This is in case the file is too big, if not, we can cache the file.
//	!				The problem is if the file asked is too big and with the current implementation it will be loaded in memory. A file of 1 GB is not aceptable.

//	!	IMPORTANT	In read_client i have to check for a new header in the buffer and then process the request. After that continue reading. (really?)
//	!				So, i need a function to check if a header is in the buffer and where start a header (ignoring the first header, of course)

//	TODO	uploads es necesario?
//	TODO	Verifica el tipo MIME del archivo y el nombre del archivo para asegurarte de que coincide con el tipo esperado.
//	TODO	Limita el tamaño máximo de las solicitudes HTTP. Esto puede prevenir que un atacante agote los recursos del servidor con solicitudes grandes.

#pragma region Comunications

	#pragma region READ

		#pragma region Client

			int Net::read_client(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));
				char peek_buffer[CHUNK_SIZE + 1];	memset(peek_buffer, 0, sizeof(peek_buffer));

				ssize_t bytes_peek = recv(event->fd, peek_buffer, CHUNK_SIZE + 1, MSG_PEEK);
				//if (bytes_peek <= 0) {event->client->remove(); return (1); }							//	Recv error or empty (empty = close client)
				
				event->client->update_last_activity();

				ssize_t bytes_read = recv(event->fd, buffer, CHUNK_SIZE, 0);
				if (bytes_read > 0) {
					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);

					Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
					read_bytes+= bytes_read;
					Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

					if (static_cast<size_t>(bytes_peek) <= CHUNK_SIZE) {
						event->request = std::string(event->read_buffer.begin(), event->read_buffer.end());
						event->read_buffer.clear();
						event->client->total_requests++;

						process_request(event);
					}
				} else if (bytes_read <= 0) {															//	Read error or empty (empty = close client)
					if (get_event(event->fd)) event->client->remove();
					if (bytes_read < 0) Log::log(RED500 "Comunication failed with " BLUE400 + event->client->IP + NC, Log::BOTH_ERROR, event->socket->VServ);
					return (1);
				}

				return (0);
			}

		#pragma endregion

		#pragma region Data

			int Net::read_data(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));

				ssize_t bytes_read = read(event->fd, buffer, CHUNK_SIZE);
				if (bytes_read > 0) {
					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);
					event->data_size += bytes_read;

					if (event->data_size >= event->max_data_size) {
						std::string data(event->read_buffer.begin(), event->read_buffer.end());

						if (event->type == DATA && event->no_cache == false) cache.add(event->path, data);

						EventInfo * c_event = get_event(event->client->fd);
						remove_event(event->fd);

						process_data(c_event, data);
						return (1);
					}
				} else if (bytes_read <= 0) { remove_event(event->fd); return (1); }							//	Read error or empty

				size_t chunk = std::min(event->max_data_size - event->data_size, CHUNK_SIZE);
				if (splice(event->pipe[0], NULL, event->pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) { remove_event(event->fd); return (1); }

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region WRITE

		#pragma region Client

			void Net::write_client(EventInfo *event) {
				if (!event) return;

				event->client->update_last_activity();

				if (!event->write_buffer.empty()) {

					size_t buffer_size = event->write_buffer.size();
					size_t chunk_size = std::min(buffer_size, static_cast<size_t>(CHUNK_SIZE));
					
					ssize_t bytes_written = write(event->fd, event->write_buffer.data(), chunk_size);

					if (bytes_written > 0) {
						event->write_buffer.erase(event->write_buffer.begin(), event->write_buffer.begin() + bytes_written);

						Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
						write_bytes+= bytes_written;
						Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

					} else if (bytes_written < 0) { event->client->remove(); return; }					//	write error
				}
				
				if (event->write_buffer.empty()) {
					epoll_set(event->fd, true, false);

					long MaxRequests = KEEP_ALIVE_TIMEOUT;

					if (Settings::global.get("keepalive_requests") != "") Utils::stol(Settings::global.get("keepalive_requests"), MaxRequests);
					Log::log("GET", "/", 200, write_bytes, "250", event->client->IP, event->socket->VServ, event->vserver_data);
					if (event->close || event->client->total_requests >= MaxRequests) event->client->remove();
				}
			}

		#pragma endregion

	#pragma endregion

	#pragma region PROCESS

		#pragma region Request

			void Net::process_request(EventInfo * event) {
				if (!event) return;

				//	if (body > max_body_size) return lo que sea

				// std::istringstream request_stream(request); std::string line;
				// if (std::getline(request_stream, line)) Utils::trim(line);
				// Log::log(line, Log::BOTH_ACCESS);
				//Log::log("Prueba", Log::BOTH_ACCESS);
				process_response(event);																//	ELIMINAR
			}

		#pragma endregion

		#pragma region Response

			void Net::process_response(EventInfo * event) {
				if (!event) return;

				event->close = true;
				Cache::CacheInfo * fcache = cache.get("index.html");
				if (fcache) {
					process_data(get_event(event->fd), fcache->content);
					return;
				}

				int fd = open("index.html", O_RDONLY);
				if (fd == -1) { return; }
				size_t filesize = Utils::filesize(fd);
				if (filesize == std::string::npos) { close(fd); return; }

				EventInfo event_data(fd, DATA, NULL, event->client);

				event_data.path = "index.html";
				event_data.no_cache = false;
				if (pipe(event_data.pipe) == -1) { remove_event(event_data.fd); return; }					//	Create pipe
				event_data.data_size = 0;
				event_data.max_data_size = filesize;
				size_t chunk = std::min(event_data.max_data_size, CHUNK_SIZE);
				if (splice(event_data.fd, NULL, event_data.pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) { close(event_data.fd); close(event_data.pipe[0]); close(event_data.pipe[1]); return; }
				std::swap(event_data.fd, event_data.pipe[0]);
				events[event_data.fd] = event_data;
				if (epoll_add(event_data.fd, true, false) == -1) { remove_event(event_data.fd); return; }

				return;
			}

		#pragma endregion

		#pragma region Data

			void Net::process_data(EventInfo * event, std::string data) {
				if (!event) return;

				// std::istringstream request_stream(data); std::string line;
				// if (std::getline(request_stream, line)) Utils::trim(line);
				// Log::log(line, Log::BOTH_ACCESS);
				std::string response = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"Content-Length: " + Utils::ltos(data.size()) + "\r\n"
					"Connection: keep-alive\r\n"
					"\r\n" + data;

				event->write_buffer.insert(event->write_buffer.end(), response.begin(), response.end());
				epoll_set(event->fd, true, true);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Resolve Request

	//	Return structure with info about the result
	void Net::resolve_request(const std::string host, const std::string method, std::string path, EventInfo * event) {
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

#pragma endregion

//	void Net::process_request(EventInfo * event, std::string request)
//
//	Esta es la funcion de entrada para el procesado de lo que pide el cliente
//	event es un puntero al EventInfo del cliente. 
//	El EventInfo se guarda en Net::events y es un map de FD - EventInfo
//	Para ver la implentacion de EventInfo, está en Net.hpp
//
//	request es un string con el contenido de la peticion.
//	Esto incluye el header y body.
//	Tu me pasarias a la funcion intermediaria el metodo (GET, POST, etc...), la ruta (/folder por ejemplo) y
//	el * event (para obtener la ip del cliente).
//
//	Eso te devolverá una estructura que todavia no esta hecha, pero que te dará la información necesaria para
//	construir la respuesta.