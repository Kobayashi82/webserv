/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Communication.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 09:32:08 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/07 14:36:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Thread.hpp"
#include "Client.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Protocol.hpp"
#include "Communication.hpp"

#pragma region Variables

	std::list<Client>	Communication::clients;															//	List of all Client objects
	Cache				Communication::cache(600, 100, 5);												//	Used to store cached data, such as files or HTML responses.	(arguments: expiration in seconds, max entries, max content size in MB)

	int					Communication::total_clients;													//	Total number of clients conected
	size_t				Communication::read_bytes;														//	Total number of bytes downloaded by the server
	size_t				Communication::write_bytes;														//	Total number of bytes uploaded by the server

	const size_t		Communication::CHUNK_SIZE = 4096;												//	Size of the buffer for read and write operations
	const size_t		Communication::HEADER_MAXSIZE = 2048;											//	Maximum size allowed for the header (8 KB by default)

#pragma endregion

#pragma region Communications

	#pragma region CLIENT

		#pragma region Read

			int Communication::read_client(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));															//	Initialize buffer
				char peek_buffer[CHUNK_SIZE + 1];	memset(peek_buffer, 0, sizeof(peek_buffer));												//	Initialize peek buffer

				ssize_t bytes_peek = recv(event->fd, peek_buffer, CHUNK_SIZE + 1, MSG_PEEK);													//	Peek chunk + 1 byte to check if there are more data
				ssize_t bytes_read = recv(event->fd, buffer, CHUNK_SIZE, 0);																	//	Read a chunk

			//	Read some data
				if (bytes_read > 0) {

					event->client->update_last_activity();																						//	Reset client timeout

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);											//	Store the data read into 'read_buffer'
					Thread::inc_size_t(Display::mutex, read_bytes, bytes_read);																	//	Increase total bytes read

				//	Needs to get the header
					if (event->header == "") {

						int result = Protocol::parse_header(event);																				//	Try to parse the header (maybe the header is not there yet)
						if (result == 0) {																										//	There is a header
							if (event->header.size() > HEADER_MAXSIZE) {																		//	Header too big, return error
								event->header_map["Connection"] = "close";																		//	Set 'Connection' to close
								event->header_map["Write-Only"] = "true";																		//	Don't read from the client anymore
								Epoll::set(event->fd, false, false);																			//	Close read and write monitor for EPOLL
								Protocol::check_code(event, true, "431");																		//	Request Header Fields Too Large
								return (1);
							}
							event->read_size = event->read_buffer.size() - event->header.size();												//	Set 'read_size'
							event->read_buffer.erase(event->read_buffer.begin(), event->read_buffer.begin() + event->header.size());			//	Remove the header from 'read_buffer'
							Protocol::process_request(event);																					//	Process the request

						} else if (result == 2) { event->client->remove(); return (1); }														//	There is a header, but something went wrong

					} else event->read_size += bytes_read;																						//	Increase 'read_size'

				//	If 'read_size' is greater than 'body_maxsize'
					if (event->body_maxsize > 0 && event->read_size >= event->body_maxsize) {
						event->header_map["Connection"] = "close";																				//	Set 'Connection' to close
						event->header_map["Write-Only"] = "true";																				//	Don't read from the client anymore
						Epoll::set(event->fd, false, false);																					//	Close read and write monitor for EPOLL
						if (event->response_method == "CGI") {
							if (event->pid != 0) { kill(event->pid, SIGKILL); event->pid = 0; }
							Event::remove(event->cgi_fd);		event->cgi_fd = -1;
							Event::remove(event->cgi_read_fd);	event->cgi_read_fd = -1;
						}
						Protocol::check_code(event, true, "413");																				//	Payload Too Large
						return (1);
					}

				//	Write to a CGI
					if (event->response_method == "CGI") {
						EventInfo * cgi_event = Event::get(event->cgi_fd);																		//	Get the CGI's event
						if (cgi_event)
							cgi_event->write_buffer.insert(cgi_event->write_buffer.end(), buffer, buffer + bytes_read);							//	Copy the data to the 'write_buffer' of the CGI
					}

				//	Clear 'read_buffer' (Not needed if not a CGI)
					if (!event->response_method.empty()) event->read_buffer.clear();

				//	No more data coming
					if (static_cast<size_t>(bytes_peek) <= CHUNK_SIZE) {
						if (event->response_method == "CGI") {
							EventInfo * cgi_event = Event::get(event->cgi_fd);																	//	Get the CGI's event
							if (cgi_event) cgi_event->write_info = 3;																			//	Set a flag (no more data)
						} return (1);
					}

			//	No data (usually means a client disconected)
				} else if (bytes_read == 0) {
					event->client->remove(); return (1);																						//	Remove the client

			//	Error reading
				} else if (bytes_read == -1) {
					Log::log(RED500 "Communication failed with " BLUE400 + event->client->ip + NC, Log::BOTH_ERROR, event->socket->VServ);		//	Log message
					event->client->remove(); return (1);																						//	Remove the client
				}		

				return (0);
			}

		#pragma endregion

		#pragma region Write

			void Communication::write_client(EventInfo * event) {
				if (!event) return;

			//	There are data, send it
				if (!event->write_buffer.empty()) {
					event->client->update_last_activity();																										//	Reset client timeout

					size_t buffer_size = event->write_buffer.size();
					size_t chunk = std::min(buffer_size, static_cast<size_t>(CHUNK_SIZE));																		//	Set the size of the chunk

					ssize_t bytes_written = send(event->fd, event->write_buffer.data(), chunk, MSG_DONTWAIT);													//	Send the data

				//	Sent some data
					if (bytes_written > 0) {

						event->write_size += bytes_written;																										//	Increase 'write_size'
						Thread::inc_size_t(Display::mutex, write_bytes, bytes_written);																			//	Increase total bytes written
						event->write_buffer.erase(event->write_buffer.begin(), event->write_buffer.begin() + bytes_written);									//	Remove the data sent from 'write_buffer'

				//	No writing if 'write_buffer' is empty, so this must be an error
					} else if (bytes_written == 0) { event->client->remove(); return;																			//	Remove the client

				//	Error writing
					} else if (bytes_written == -1) { event->client->remove(); return; }																		//	Remove the client
				}

			//	All data has been sent
				if ((event->write_info == 0 && event->write_maxsize > 0 && event->write_size >= event->write_maxsize)
					|| (event->write_info == 3 && event->write_buffer.empty())) {

				//	Log connection
					std::string time = Utils::ltos(Settings::timer.elapsed_milliseconds(event->response_time));													//	Get the time to process the request in milliseconds
					gettimeofday(&event->response_time, NULL);																									//	Reset response time
					
					Log::log(	"TRF|" + event->header_map["Method"] + "|" + event->header_map["Path"] + "|" + event->response_map["Code"] + "|" + Utils::ltos(event->response_size) +			//	Log the client request
								"|" + time + "|" + event->client->ip, Log::BOTH_ACCESS, event->VServ, event->vserver_data);

				//	Check close connection
					long MaxRequests = Settings::KEEP_ALIVE_TIMEOUT;
					if (Settings::global.get("keepalive_requests") != "") Utils::stol(Settings::global.get("keepalive_requests"), MaxRequests);					//	Get the maximum request allowed
					if (event->header_map["Connection"] == "close" || event->client->total_requests + 1 >= MaxRequests) { event->client->remove(); return; }	//	Close the connection if client ask or max request reach

				//	Clear connection info					
					event->client->total_requests++;																											//	Increase total_requests
					event->header = ""; event->header_map.clear(); event->response_map.clear();																	//	Clear header and maps
					Epoll::set(event->fd, true, false);																											//	Monitor read events only in EPOLL 
				}
			}

		#pragma endregion

	#pragma endregion

	#pragma region DATA

		#pragma region Read

			int Communication::read_data(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));														//	Initialize buffer

			//	Read a chunk from the FD
				ssize_t bytes_read = read(event->fd, buffer, CHUNK_SIZE);

			//	Read some data
				if (bytes_read > 0) {

					Event::update_last_activity(event->fd);																						//	Reset event timeout

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);											//	Store the data read into 'read_buffer'
					event->read_size += bytes_read;																								//	Increase 'read_size'

					if (event->read_size == static_cast<size_t>(bytes_read)) {
						EventInfo * c_event = Event::get(event->client->fd);
						if (c_event && Epoll::set(c_event->fd, !(c_event->header_map["Write-Only"] == "true"), true) == -1) {					//	Set EPOLL to monitor write events for the client
							c_event->client->remove(); return (1);
						}
					}

				//	No cache allowed
					if (event->no_cache == true) {
						EventInfo * c_event = Event::get(event->client->fd);																	//	Get the client's event
						if (c_event) {
							c_event->write_buffer.insert(c_event->write_buffer.end(), buffer, buffer + bytes_read);								//	Send the data to client's 'write_buffer'
							event->read_buffer.clear();

						//	All data has been read
							if (event->read_size >= event->read_maxsize) {
								c_event->write_info = 3;																						//	Set a flag (no more data)
								Event::remove(event->fd); return (1);																			//	Remove the event
							}
						} else { Event::remove(event->fd); return (1); }

				//	Cache allowed and all data has been read
					} else if (event->read_size >= event->read_maxsize) {
						std::string data(event->read_buffer.begin(), event->read_buffer.end());													//	Create a string with the data
						EventInfo * c_event = Event::get(event->client->fd);																	//	Get the client's event
						if (c_event) {
							cache.add(event->read_path, data, c_event->response_map["Last-Modified"], c_event->mod_time);						//	Add the data to the cache
							Communication::cache.remove_caching(event->read_path);																//	Remove the file from the caching list
							c_event->write_info = 3;																							//	Set a flag (no more data)
							c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());	//	Send the data to client's 'write_buffer'
						}
						Event::remove(event->fd); return (1);																					//	Remove the event
					}

			//	No data
				} else if (bytes_read == 0) {
					EventInfo * c_event = Event::get(event->client->fd);																		//	Get the client's event
					if (c_event) {
						c_event->read_info = 3;																									//	Set a flag (no more data)
						if (event->no_cache == false)
							c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());	//	Send the data to client's 'write_buffer'
					}
					Event::remove(event->fd); return (1);																						//	Remove the event

			//	Error reading
				} else if (bytes_read == -1) {
					EventInfo * c_event = Event::get(event->client->fd);																		//	Get the client's event
					if (c_event) {
						c_event->read_info = 3;																										//	Set a flag (no more data)
						if (event->no_cache == false)
							c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());		//	Send the data to client's 'write_buffer'
						Protocol::check_code(c_event, true, "500");
					}
					Event::remove(event->fd); return (1);																						//	Remove the event
				}

			//	Send more data from the file to the pipe (EPOLL is monitoring the pipe)
				size_t chunk = CHUNK_SIZE;
				if (event->read_maxsize > 0) chunk = std::min(event->read_maxsize - event->read_size, CHUNK_SIZE);								//	Set the size of the chunk

				if (splice(event->pipe[0], NULL, event->pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) {											//	Move data with splice from the file FD to the pipe
					Event::remove(event->fd); return (1);																						//	Remove the event
				}

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region CGI

		#pragma region Read

			int Communication::read_cgi(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));											//	Initialize buffer

			//	Read a chunk from the FD
				ssize_t bytes_read = read(event->fd, buffer, CHUNK_SIZE);

			//	Read some data
				if (bytes_read > 0) {

					Event::update_last_activity(event->fd);																		//	Reset event timeout

					EventInfo * c_event = Event::get(event->client->fd);														//	Get the client's event
					if (!c_event) { Event::remove(event->fd); return (1); }														//	Client's event is missing

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);							//	Store the data read into 'read_buffer'
					event->read_size += bytes_read;																				//	Increase 'read_size'

					if (event->read_info == 1 || event->read_info == 2)
						c_event->response_size += event->read_size - event->header.size();										//	Increase 'read_size' only if 'read_info' equal to 1 or 2

				//	Needs to get the header
					if (event->read_info == 0 && event->read_maxsize == 0 && event->header == "") {
						size_t content_length = 0;
						int result = Protocol::parse_header(event);																//	Try to parse the header (maybe the header is not there yet)
						if (result == 0) {																						//	There is a header
							event->read_size += event->read_buffer.size() - bytes_read;
							bytes_read = event->read_buffer.size();

							c_event->response_map["Protocol"] = event->header_map["Protocol"];
							c_event->response_map["Code"] = event->header_map["Code"];
							c_event->response_map["Code-Description"] = event->header_map["Code-Description"];
							 c_event->response_map["Connection"] = event->header_map["Connection"];
							if (event->header_map["Transfer-Encoding"] == "chunked") {											//	Get 'Transfer-Encoding'
								c_event->response_size = event->read_size - event->header.size();								//	Set 'response_size'
								event->read_info = 2;																			//	Set a flag (content is chunked)

							} else if (Utils::stol(event->header_map["Content-Length"], content_length)) {						//	There is no 'Content-Length'
								c_event->header_map["Connection"] = "close";													//	Set 'Connection' to close
								c_event->response_size = event->read_size - event->header.size();								//	Set 'response_size'
								event->read_info = 1;																			//	Set a flag (unkown content length)

							} else {																							//	There is 'Content-Length'
								event->read_maxsize = event->header.size() + content_length;									//	Set 'response_size'
								c_event->response_size = content_length;														//	Set 'response_size'
							}

							c_event->write_info = event->read_info;																//	Set client's 'write_info'
							c_event->write_maxsize = event->read_maxsize;														//	Set client's 'write_maxsize'

							if (Epoll::set(c_event->fd, !(c_event->header_map["Write-Only"] == "true"), true) == -1) {			//	Set EPOLL to monitor write events for the client
								c_event->client->remove(); return (1);
							}
						} else if (result == 2) {
							if (c_event->cgi_fd != -1) { Event::remove(c_event->cgi_fd); c_event->cgi_fd = -1; }				//	There is a header, but something went wrong
							Protocol::check_code(c_event, true, "500"); Event::remove(event->fd); return (1);
						} else if (result == 3) {
							if (c_event->cgi_fd != -1) { Event::remove(c_event->cgi_fd); c_event->cgi_fd = -1; }				//	There is a header, but no 'c_event'
							Protocol::check_code(c_event, true, "500"); Event::remove(event->fd); return (1);
						} else if (result == 4) { Event::remove(event->fd); return (1); }										//	There is a header, but the response is determined by the server, ignore the CGI
					}

				//	Send the data to client's 'write_buffer'
					if (!event->header.empty())
						c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.begin() + bytes_read);

				//	All data has been read (content length)
					if (event->read_info == 0 && event->read_maxsize > 0 && event->read_size >= event->read_maxsize) {
						c_event->write_info = 3;																				//	Set a flag (no more data)
						Event::remove(event->fd); return (1);																	//	Remove the event
					}

				//	All data has been read (chunked)
					if (event->read_info == 2) {
						static std::string last_data;
						if (last_data.size() > 7) last_data = last_data.substr(last_data.size() - 7);
						last_data += std::string(event->read_buffer.begin(), event->read_buffer.end());							//	Keep the last 7 bytes of data
						
						if (last_data.substr(std::min(last_data.size() - 7, last_data.size())) == "\r\n0\r\n\r\n") {			//	If last 7 bytes are the end of the chunks
							c_event->write_info = 3;																			//	Set a flag (no more data)
							Event::remove(event->fd); return (1);																//	Remove the event
						}
					}

				//	All data has been read (unkown content length)
					if (static_cast<size_t>(bytes_read) < CHUNK_SIZE && (event->read_info == 1 || event->header == "")) {
						c_event->header_map["Connection"] = "close";															//	Set 'Connection' to close
						c_event->write_info = 3;																				//	Set a flag (no more data)
						Event::remove(event->fd); return (1);																	//	Remove the event
					}

					if (event->header != "" || event->read_info > 0) event->read_buffer.clear();

			//	No data
				} else if (bytes_read == 0 && event->read_info == 1) {
					EventInfo * c_event = Event::get(event->client->fd);														//	Get the client's event
					if (c_event) {
						c_event->header_map["Connection"] = "close";															//	Set 'Connection' to close
						c_event->write_info = 3;																				//	Set a flag (no more data)
						if (c_event->cgi_fd != -1) { Event::remove(c_event->cgi_fd); c_event->cgi_fd = -1; }
						Protocol::check_code(c_event, true, "500");
					}
					Event::remove(event->fd); return (1);																		//	Remove the event

			//	Error reading
				} else if (bytes_read == -1) {
					EventInfo * c_event = Event::get(event->client->fd);														//	Get the client's event
					if (c_event) {
						c_event->header_map["Connection"] = "close";															//	Set 'Connection' to close
						c_event->write_info = 3;																				//	Set a flag (no more data)
						if (c_event->cgi_fd != -1) { Event::remove(c_event->cgi_fd); c_event->cgi_fd = -1; }
						Protocol::check_code(c_event, true, "500");
					}
					Event::remove(event->fd); return (1);																		//	Remove the event
				}

				return (0);
			}

		#pragma endregion

		#pragma region Write

			void Communication::write_cgi(EventInfo * event) {
				if (!event) return;

			//	There are data in the 'write_buffer'
				if (!event->write_buffer.empty()) {

					Event::update_last_activity(event->fd);																		//	Reset event timeout

					size_t buffer_size = event->write_buffer.size();
					size_t chunk = CHUNK_SIZE;
					if (buffer_size > 0) chunk = std::min(buffer_size, static_cast<size_t>(CHUNK_SIZE));						//	Set the size of the chunk
					
					ssize_t bytes_written = write(event->fd, event->write_buffer.data(), chunk);								//	Send the data

				//	Sent some data
					if (bytes_written > 0) {
						event->write_buffer.erase(event->write_buffer.begin(), event->write_buffer.begin() + bytes_written);	//	Remove the data sent from 'write_buffer'

						event->write_size += bytes_written;																		//	Increase 'write_size'

				//	No writing if 'write_buffer' is empty, so this must be an error
					} else if (bytes_written == 0) { Event::remove(event->fd); return;											//	Remove the event

				//	Error writing
					} else if (bytes_written == -1) { Event::remove(event->fd); return; }										//	Remove the event
				}

				//	All data has been sent
				if ((event->write_info == 0 && event->write_size >= event->write_maxsize)
					|| (event->write_info == 3 && event->write_buffer.empty())) {
					Event::remove(event->fd);																					//	Remove the event
				}
			}

		#pragma endregion

	#pragma endregion

#pragma endregion
