/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Comunications.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 09:32:08 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/28 21:20:35 by vzurera-         ###   ########.fr       */
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

//	!	IMPORTANT	In read_client i have to check for a new header in the buffer and then process the request. After that continue reading.
//	!				So, i need a function to check if a header is in the buffer and where start a header (ignoring the first header, of course)

#pragma region Comunications

	#pragma region READ

		#pragma region Client

			int Net::read_client(EventInfo * event) {
				char buffer[EPOLL_BUFFER_SIZE];				memset(buffer, 0, sizeof(buffer));
				char peek_buffer[EPOLL_BUFFER_SIZE + 1];	memset(peek_buffer, 0, sizeof(peek_buffer));

				ssize_t bytes_peek = recv(event->fd, peek_buffer, EPOLL_BUFFER_SIZE + 1, MSG_PEEK);
				if (bytes_peek <= 0) { event->client->remove(); return (1); }							//	Recv error or empty (empty = close client)
				
				event->client->update_last_activity();

				ssize_t bytes_read = recv(event->fd, buffer, EPOLL_BUFFER_SIZE, 0);
				if (bytes_read > 0) {
					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);

					Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
					read_bytes+= bytes_read;
					Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

					//	if (event->client->read_buffer.size() >= max_request_size) return lo que sea

					if (static_cast<size_t>(bytes_peek) <= EPOLL_BUFFER_SIZE) {
						std::string request(event->read_buffer.begin(), event->read_buffer.end());
						event->read_buffer.clear();
						event->client->total_requests++;

						process_request(event, request);
					}
				} else if (bytes_read <= 0) { event->client->remove(); return (1); }					//	Read error or empty (empty = close client)

				return (0);
			}

		#pragma endregion

		#pragma region Data

			int Net::read_data(EventInfo * event) {
				char buffer[EPOLL_BUFFER_SIZE];				memset(buffer, 0, sizeof(buffer));

				ssize_t bytes_read = read(event->fd, buffer, EPOLL_BUFFER_SIZE);
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

				size_t chunk = std::min(event->max_data_size - event->data_size, EPOLL_BUFFER_SIZE);
				if (splice(event->pipe[0], NULL, event->pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) { remove_event(event->fd); return (1); }

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region WRITE

		#pragma region Client

			void Net::write_client(EventInfo *event) {
				event->client->update_last_activity();

				if (!event->write_buffer.empty()) {

					size_t buffer_size = event->write_buffer.size();
					size_t chunk_size = std::min(buffer_size, static_cast<size_t>(EPOLL_BUFFER_SIZE));
					
					ssize_t bytes_written = write(event->fd, event->write_buffer.data(), chunk_size);

					if (bytes_written > 0) {
						event->write_buffer.erase(event->write_buffer.begin(), event->write_buffer.begin() + bytes_written);

						Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
						write_bytes+= bytes_written;
						Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

					} else if (bytes_written < 0) { event->client->remove(); return; }					// write error
				}
				
				if (event->write_buffer.empty()) {
					epoll_set(event->fd, true, false);

					long MaxRequests = KEEP_ALIVE_TIMEOUT;

					if (Settings::global.get("keepalive_requests") != "") Utils::stol(Settings::global.get("keepalive_requests"), MaxRequests);
					if (event->client->total_requests >= MaxRequests) event->client->remove();
				}
			}

		#pragma endregion

	#pragma endregion

	//	HERE WORK MUST BE DONE (PERSON 1)
	#pragma region PROCESS

		#pragma region Request

			void Net::process_request(EventInfo * event, std::string request) {
				//	Diferenciar entre dos request unidas. Se busca un header y eso determina el inicio de otro response?
				//	Si tenemos body_length leemos solo hasta ahi y el resto va de vuelta al buffer

				//	if (body > max_body_size) return lo que sea

				// std::istringstream request_stream(request); std::string line;
				// if (std::getline(request_stream, line)) Utils::trim(line);
				// Log::log(line, Log::BOTH_ACCESS);
				(void) request;
				process_response(event);																//	ELIMINAR
			}

		#pragma endregion

		#pragma region Response

#include <sys/stat.h>

			void Net::process_response(EventInfo * event) {
				// Crear una respuesta HTTP básica
				// std::string body = "<html><body><h1>Hello, World!</h1></body></html>";
				// std::string response = 
				// 	"HTTP/1.1 200 OK\r\n"
				// 	"Content-Type: text/html\r\n"
				// 	"Content-Length: " + Utils::ltos(body.size()) + "\r\n"
				// 	"Connection: keep-alive\r\n"
				// 	//"Connection: close\r\n"
				// 	"\r\n" + body;

				// event->client->write_buffer.insert(event->client->write_buffer.end(), response.begin(), response.end());
				// epoll_set(event, true, true);

				Cache::CacheInfo * fcache = cache.get("index.html");
				if (fcache) {
					process_data(get_event(event->fd), fcache->content);
					return;
				}

				int fd = open("index.html", O_RDONLY);
				if (fd == -1) { return; }
				struct stat file_stat; if (fstat(fd, &file_stat) == -1) { close(fd); return; }			//	Size of the file

				EventInfo event_data(fd, DATA, NULL, event->client);

				event_data.path = "index.html";
				event_data.no_cache = true;
				if (pipe(event_data.pipe) == -1) { remove_event(event_data.fd); return; }					//	Create pipe
				event_data.data_size = 0;
				event_data.max_data_size = file_stat.st_size;
				size_t chunk = std::min(event_data.max_data_size, EPOLL_BUFFER_SIZE);
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
					//"Connection: close\r\n"
					"\r\n" + data;

				event->write_buffer.insert(event->write_buffer.end(), response.begin(), response.end());
				epoll_set(event->fd, true, true);
			}

		#pragma endregion

	#pragma endregion
	//	HERE WORK MUST BE DONE (PERSON 1)

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
