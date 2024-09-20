/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Comunication.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 09:32:08 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/20 17:07:03 by vzurera-         ###   ########.fr       */
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

#pragma region Variables

	std::list<Client>	Comunication::clients;															//	List of all Client objects
	Cache				Comunication::cache(600, 100, 10);												//	Used to store cached data, such as files or HTML responses.	(arguments: expiration in seconds, max entries, max content size in MB)

	int					Comunication::total_clients;													//	Total number of clients conected
	size_t				Comunication::read_bytes;														//	Total number of bytes downloaded by the server
	size_t				Comunication::write_bytes;														//	Total number of bytes uploaded by the server

	const size_t		Comunication::CHUNK_SIZE = 4096;												//	Size of the buffer for read and write operations

#pragma endregion

#pragma region Parse Header

	int parse_header(EventInfo * event) {
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
			pos = line.find(':');																			//	Find ':' to split Key - Value
			if (pos != std::string::npos) event->header_map[line.substr(0, pos)] = line.substr(pos + 2);	//	Add the Key - Value to 'header_map'
		}

		return (0);
	}

#pragma endregion

#pragma region Comunications

	#pragma region CLIENT

		#pragma region Read

			int Comunication::read_client(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));					//	Initialize buffer
				char peek_buffer[CHUNK_SIZE + 1];	memset(peek_buffer, 0, sizeof(peek_buffer));		//	Initialize peek buffer

				ssize_t bytes_peek = recv(event->fd, peek_buffer, CHUNK_SIZE + 1, MSG_PEEK);			//	Peek chunk + 1 byte to check if there are more data
				ssize_t bytes_read = recv(event->fd, buffer, CHUNK_SIZE, 0);							//	Read a chunk

				if (bytes_read > 0) {																	//	Read some data

					event->client->update_last_activity();												//	Reset client timeout
					Event::update_last_activity(event->fd);												//	Reset event timeout

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);	//	Store the data read into read_buffer

					Thread::inc_size_t(Display::mutex, read_bytes, bytes_read);							//	Increase total bytes read

					if (event->header == "") {															//	If no header yet, try to get one
						int result = parse_header(event);												//	Parse the header
						if (result == 0) {																//	There is a header
							event->body_size = event->read_buffer.size() - event->header.size();		//	Set body_size of the current request
							//event->header_map["Cache-Control"] = "no_cache";
							//	En process_request check content_lentgh and body_maxsize
							process_request(event);														//	Process the request
						} else if (result == 2) {														//	There is a header, but something went wrong
							event->client->remove(); return (1);										//	Error, connection close
						}
					} else event->body_size += bytes_read;												//	Increase body_size

					if (event->body_maxsize > 0 && event->body_size > event->body_maxsize) {			//	If body_size is greater than body_maxsize
						event->header_map["Connection"] = "close";										//	Set the conection to close
						event->header_map["Write_Only"] = "true";										//	Don't read from the client anymore
						Epoll::set(event->fd, false, false);											//	Close read and write monitor for EPOLL
						//	Return error;																//	Return error
						return (1);
					}

					if (event->method == "CGI") {														//	Write to a cgi
						EventInfo * cgi_event = Event::get(event->cgi_fd);								//	Get the event of the CGI
						if (!cgi_event) event->client->remove(); return (1);							//	If no CGI event, error, close connection (this should not happen)
						cgi_event->write_buffer.insert(cgi_event->write_buffer.end(), buffer, buffer + bytes_read);	//	Copy the data to the write_buffer of the CGI
					}

					if (!event->method.empty()) event->read_buffer.clear();								//	Clear read_buffer (Not needed if not a CGI)

					if (static_cast<size_t>(bytes_peek) <= CHUNK_SIZE) {								//	No more data coming
						if (event->method == "CGI") {
							EventInfo * cgi_event = Event::get(event->cgi_fd);							//	Get the event of the CGI
							if (!cgi_event) cgi_event->file_info = 2;									//	Send a flag to the CGI that no more data is coming
						}
						return (1);
					}

				} else if (bytes_read == 0) {															//	No data (usually means a client disconected)
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

				if (!event->write_buffer.empty()) {														//	There are data in the write_buffer

					event->client->update_last_activity();												//	Reset client timeout
					Event::update_last_activity(event->fd);												//	Reset event timeout

					size_t buffer_size = event->write_buffer.size();									//	Set the size of the chunk
					size_t chunk = CHUNK_SIZE;
					if (buffer_size > 0) chunk = std::min(buffer_size, static_cast<size_t>(CHUNK_SIZE));
					
					ssize_t bytes_written = write(event->fd, event->write_buffer.data(), chunk);								//	Send the data

					if (bytes_written > 0) {																					//	Some data is sent
						event->write_buffer.erase(event->write_buffer.begin(), event->write_buffer.begin() + bytes_written);	//	Remove the data sent from write_buffer

						event->file_read += bytes_written;																		//	Increase file_read

						Thread::inc_size_t(Display::mutex, write_bytes, bytes_written);											//	Increase total bytes written

					} else { event->client->remove(); return; }																	//	Error writing
				}

				if ((event->file_info == 0 && event->file_read >= event->file_size) || (event->file_info == 2 && event->write_buffer.empty())) {	//	All data has been sent
					long MaxRequests = Settings::KEEP_ALIVE_TIMEOUT;
					if (Settings::global.get("keepalive_requests") != "") Utils::stol(Settings::global.get("keepalive_requests"), MaxRequests);		//	Get the maximum request allowed

					Log::log("GET", "/", 200, write_bytes, "250", event->client->ip, event->socket->VServ, event->vserver_data);					//	Log the client request

					if (event->header_map["Connection"] == "close" || event->client->total_requests + 1 >= MaxRequests)								//	Close the connection if client ask or max request reach
						event->client->remove();
					else {
						Epoll::set(event->fd, true, false);																							//	Monitor read events only in EPOLL 
						event->client->total_requests++;																							//	Increase total_requests
					}
				}

			}

		#pragma endregion

	#pragma endregion

	#pragma region DATA

		#pragma region Read

			int Comunication::read_data(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));					//	Initialize buffer

				ssize_t bytes_read = read(event->fd, buffer, CHUNK_SIZE);								//	Read a chunk

				if (bytes_read > 0) {																	//	Read some data

					Event::update_last_activity(event->fd);												//	Reset event timeout

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);	//	Store the data read into read_buffer
					event->file_read += bytes_read;														//	Increase file_read

					if (event->no_cache == true) {																							//	No cache allowed
						EventInfo * c_event = Event::get(event->client->fd);																//	Get the event of the client
						c_event->write_buffer.insert(c_event->write_buffer.end(), buffer, buffer + bytes_read);								//	Copy the data to the write_buffer of the client
						if (event->file_read >= event->file_size) {																			//	All data has been read
							c_event->file_info = 2;																							//	Send a flag to the client that no more data is coming
							Event::remove(event->fd); return (1);																			//	Remove the event
						}

					} else if (event->file_read >= event->file_size) {																		//	Cache allowed and all data has been read
						std::string data(event->read_buffer.begin(), event->read_buffer.end());												//	Create a string with the data
						cache.add(event->file_path, data);																					//	Add the data to the cache

						EventInfo * c_event = Event::get(event->client->fd);																//	Get the event of the client
						c_event->file_info = 2;																								//	Send a flag to the client that no more data is coming
						c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());	//	Copy the data to the write_buffer of the client
						Event::remove(event->fd); return (1);																				//	Remove the event
					}

				} else if (bytes_read == 0) {																														//	No data
					EventInfo * c_event = Event::get(event->client->fd);																							//	Get the event of the client
					c_event->file_info = 2;																															//	Send a flag to the client that no more data is coming
					if (event->no_cache == false) c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());	//	Copy the data to the write_buffer of the client
					Event::remove(event->fd); return (1);																											//	Remove the event
				} else if (bytes_read == -1) {																														//	Error reading
					EventInfo * c_event = Event::get(event->client->fd);																							//	Get the event of the client
					c_event->file_info = 2;																															//	Send a flag to the client that no more data is coming
					if (event->no_cache == false) c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());	//	Copy the data to the write_buffer of the client
					Event::remove(event->fd); return (1);																											//	Remove the event
				}

				size_t chunk = CHUNK_SIZE;																															//	Set the size of the chunk
				if (event->file_size > 0) chunk = std::min(event->file_size - event->file_read, CHUNK_SIZE);
				if (splice(event->pipe[0], NULL, event->pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) { Event::remove(event->fd); return (1); }						//	Send more data from the file to the pipe (EPOLL is monitoring the pipe)

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region CGI

		#pragma region Read

			int Comunication::read_cgi(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));					//	Initialize buffer

				ssize_t bytes_read = read(event->fd, buffer, CHUNK_SIZE);								//	Read a chunk

				if (bytes_read > 0) {																	//	Read some data

					Event::update_last_activity(event->fd);												//	Reset event timeout

					EventInfo * c_event = Event::get(event->client->fd);								//	Get the event of the client

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);	//	Store the data read into read_buffer
					event->file_read += bytes_read;														//	Increase file_read

					if (event->file_info == 0 && event->file_size == 0) {												//	Needs to get 'content_length'
						if (event->header == "") {																		//	If no header yet, try to get one
							size_t content_length = 0;
							int result = parse_header(event);															//	Try to parse the header (maybe the header is not there yet)
							if (result == 0) {																			//	There is a header
								content_length = Utils::stol(event->header_map["Content_length"], content_length);		//	Get 'content_length'
								if (content_length == 0) event->file_info = 1;											//	If no 'content_length' set a flag
								else {
									c_event->file_info = event->file_info;												//	If 'content_length' set the flag for the client
									c_event->file_size = event->file_size;												//	If 'content_length' set the file_size for the client
								}
							} else if (result == 2) {																	//	There is a header, but something went wrong
								Event::remove(event->fd); return (1);													//	Remove the event
							}
						}
					}

					c_event->write_buffer.insert(c_event->write_buffer.end(), buffer, buffer + bytes_read);				//	Copy the data to the write_buffer of the client

					if (event->file_info == 0 && event->file_read >= event->file_size) {								//	All data has been read
						c_event->file_info = 2;																			//	Send a flag to the client that no more data is coming
						Event::remove(event->fd); return (1);															//	Remove the event
					}

				} else if (bytes_read == 0 && event->file_info == 1) {													//	No data
					Event::get(event->client->fd)->file_info = 2;														//	Send a flag to the client that no more data is coming
					Event::remove(event->fd); return (1);																//	Remove the event
				} else if (bytes_read == -1) {																			//	Error reading
					Event::get(event->client->fd)->file_info = 2;														//	Send a flag to the client that no more data is coming
					Event::remove(event->fd); return (1);																//	Remove the event
				}

				return (0);
			}

		#pragma endregion

		#pragma region Write

			void Comunication::write_cgi(EventInfo * event) {
				if (!event) return;

				if (!event->write_buffer.empty()) {																									//	There are data in the write_buffer

					Event::update_last_activity(event->fd);																							//	Reset event timeout

					size_t buffer_size = event->write_buffer.size();																				//	Set the size of the chunk
					size_t chunk = CHUNK_SIZE;
					if (buffer_size > 0) chunk = std::min(buffer_size, static_cast<size_t>(CHUNK_SIZE));
					
					ssize_t bytes_written = write(event->fd, event->write_buffer.data(), chunk);													//	Send the data

					if (bytes_written > 0) {																										//	Some data is sent
						event->write_buffer.erase(event->write_buffer.begin(), event->write_buffer.begin() + bytes_written);						//	Remove the data sent from write_buffer

						event->file_read += bytes_written;																							//	Increase file_read

					} else { Event::remove(event->fd); return; }																					//	Error writing, remove the event
				}

				if ((event->file_info == 0 && event->file_read >= event->file_size) || (event->file_info == 2 && event->write_buffer.empty())) {	//	All data has been sent
					Event::remove(event->fd);																										//	Remove the event
				}
			}

		#pragma endregion

	#pragma endregion

	#pragma region PROCESS

		#pragma region Request

			//	This temporary function process the request

			void Comunication::process_request(EventInfo * event) {
				if (!event) return;

				//	if (tipo = CGI) {
					//	create fork and pipes
					//	create cgi_event
					//	if (content_length) {
						//	cgi_event->file_size == content_length;
					//	else
						//	cgi_event->file_info == 1;
				//	}

				process_response(event);
			}

		#pragma endregion

		#pragma region Response

			//	This temporary function process the response
			//	It could be refactorized to use it as a function for serving a file

			void Comunication::process_response(EventInfo * event) {
				if (!event) return;

				if (event->header_map["Cache-Control"].empty()) {										//	If cache is allowed
					CacheInfo * fcache = cache.get("index.html");										//	Get the file from cache
					if (fcache) {																		//	If the file exist in cache
						std::string response = 															//	Create a header (it should created already)
							"HTTP/1.1 200 OK\r\n"
							"Content-Type: text/html\r\n"
							"Content-Length: " + Utils::ltos(fcache->content.size()) + "\r\n"
							"Connection: close\r\n"
							"\r\n";

						event->file_info = 0;																					//	Set some flags
						event->file_read = 0;																					//	Set some flags
						event->file_size = response.size() + fcache->content.size();											//	Set the total size of the data to be sent
						event->write_buffer.clear();																			//	Clear write_buffer
						event->write_buffer.insert(event->write_buffer.end(), response.begin(), response.end());				//	Copy the header to write_buffer
						event->write_buffer.insert(event->write_buffer.end(), fcache->content.begin(), fcache->content.end());	//	Copy the body to write_buffer
						if (Epoll::set(event->fd, true, true) == -1) {															//	Set EPOLL to monitor write events
							event->file_size = 0;																				//	If set EPOLL fails, reset the flag
							event->write_buffer.clear();																		//	and clear writte_buffer
						}

						return;
					}
				}

				int fd = open("index.html", O_RDONLY);														//	Open the file
				if (fd == -1) return;																		//	If error opening, return
				Utils::NonBlocking_FD(fd);																	//	Set the FD as non-blocking

				size_t filesize = Utils::filesize(fd);														//	Get the file size
				if (filesize == std::string::npos) { close(fd); return; }

				EventInfo event_data(fd, DATA, NULL, event->client);										//	Create the event for the DATA

				event_data.file_path = "index.html";														//	Set the name of the file
				event_data.file_size = filesize;															//	Set the size of the file
				event_data.no_cache = !event->header_map["Cache-Control"].empty();							//	Set if the file must be added to cache

				if (pipe(event_data.pipe) == -1) { close(event_data.fd); return; }							//	Create the pipe for DATA (to be used with splice)
				Utils::NonBlocking_FD(event_data.pipe[0]);													//	Set the read end of the pipe as non-blocking
				Utils::NonBlocking_FD(event_data.pipe[1]);													//	Set the write end of the pipe as non-blocking

				size_t chunk = CHUNK_SIZE;																
				if (event_data.file_size > 0) chunk = std::min(event_data.file_size, CHUNK_SIZE);			//	Set the size of the chunk
				if (splice(event_data.fd, NULL, event_data.pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) {	//	Send the first chunk of data from the file to the pipe
					close(event_data.fd); close(event_data.pipe[0]); close(event_data.pipe[1]); return;
				}
				std::swap(event_data.fd, event_data.pipe[0]);												//	Swap the read end of the pipe with the fd (this is to be consistent with the FD that EPOLL is monitoring)

				Event::events[event_data.fd] = event_data;													//	Add the DATA event to the event's list

				std::string response = 																		//	Create a header (it should created already)
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"Content-Length: " + Utils::ltos(event_data.file_size) + "\r\n"
					"Connection: close\r\n"
					"\r\n";

				event->file_info = 0;																		//	Set some flags
				event->file_read = 0;																		//	Set some flags
				event->file_size = response.size() + event_data.file_size;									//	Set the total size of the data to be sent
				event->write_buffer.clear();																//	Clear write_buffer
				event->write_buffer.insert(event->write_buffer.end(), response.begin(), response.end());	//	Copy the header to write_buffer
				if (Epoll::set(event->fd, !(event->header_map["Write_Only"] == "true"), true) == -1 || Epoll::add(event_data.fd, true, false) == -1) {	//	Set EPOLL to monitor write events for the client and read events for DATA
					event->file_size = 0;																	//	If set EPOLL fails, reset the flag,
					event->write_buffer.clear();															//	clear writte_buffer
					Event::remove(event_data.fd); return;													//	and remove the DATA event
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
