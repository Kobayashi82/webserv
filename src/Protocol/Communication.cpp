/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Communication.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 09:32:08 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/23 01:29:59 by vzurera-         ###   ########.fr       */
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
	Cache				Communication::cache(600, 100, 10);												//	Used to store cached data, such as files or HTML responses.	(arguments: expiration in seconds, max entries, max content size in MB)

	int					Communication::total_clients;													//	Total number of clients conected
	size_t				Communication::read_bytes;														//	Total number of bytes downloaded by the server
	size_t				Communication::write_bytes;														//	Total number of bytes uploaded by the server

	const size_t		Communication::CHUNK_SIZE = 4096;												//	Size of the buffer for read and write operations
	const size_t		Communication::HEADER_MAXSIZE = 8192;											//	Maximum size allowed for the header (8 KB by default)

#pragma endregion

#pragma region Communications

	#pragma region CLIENT

		#pragma region Read

			int Communication::read_client(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));					//	Initialize buffer
				char peek_buffer[CHUNK_SIZE + 1];	memset(peek_buffer, 0, sizeof(peek_buffer));		//	Initialize peek buffer

				ssize_t bytes_peek = recv(event->fd, peek_buffer, CHUNK_SIZE + 1, MSG_PEEK);			//	Peek chunk + 1 byte to check if there are more data
				ssize_t bytes_read = recv(event->fd, buffer, CHUNK_SIZE, 0);							//	Read a chunk

				if (bytes_read > 0) {																	//	Read some data

					event->client->update_last_activity();												//	Reset client timeout
					Event::update_last_activity(event->fd);												//	Reset event timeout

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);	//	Store the data read into 'read_buffer'

					Thread::inc_size_t(Display::mutex, read_bytes, bytes_read);							//	Increase total bytes read

					if (event->header == "") {															//	If no header yet, try to get one
						if (event->read_buffer.size() > HEADER_MAXSIZE)	{								//	If header is too big
							//	return error;
							return (1);																	//	Header too big, return error
						}
						int result = Protocol::parse_header(event);										//	Parse the header
						if (result == 0) {																//	There is a header
							event->body_size = event->read_buffer.size() - event->header.size();		//	Set 'body_size' of the current request
							event->write_buffer.erase(event->read_buffer.begin(), event->read_buffer.begin() + event->header.size());
							Protocol::process_request(event);											//	Process the request
						} else if (result == 2) {														//	There is a header, but something went wrong
							event->client->remove(); return (1);										//	Error, connection close
						}
					} else event->body_size += bytes_read;												//	Increase 'body_size'

					if (event->body_maxsize > 0 && event->body_size > event->body_maxsize) {			//	If 'body_size' is greater than 'body_maxsize'
						event->header_map["Connection"] = "close";										//	Set the conection to close
						event->header_map["Write_Only"] = "true";										//	Don't read from the client anymore
						Epoll::set(event->fd, false, false);											//	Close read and write monitor for EPOLL
						//	return error;																//	Body too big, return error
						return (1);
					}

					if (event->method == "CGI") {																		//	Write to a cgi
						EventInfo * cgi_event = Event::get(event->cgi_fd);												//	Get the event of the CGI
						if (!cgi_event) event->client->remove(); return (1);											//	If no CGI event, error, close connection (this should not happen)
						cgi_event->write_buffer.insert(cgi_event->write_buffer.end(), buffer, buffer + bytes_read);		//	Copy the data to the 'write_buffer' of the CGI
					}

					if (!event->method.empty()) event->read_buffer.clear();								//	Clear 'read_buffer' (Not needed if not a CGI)

					if (static_cast<size_t>(bytes_peek) <= CHUNK_SIZE) {								//	No more data coming
						if (event->method == "CGI") {
							EventInfo * cgi_event = Event::get(event->cgi_fd);							//	Get the event of the CGI
							if (!cgi_event) cgi_event->file_info = 2;									//	Send a flag to the CGI that no more data is coming
						}
						Protocol::process_request(event);
						return (1);
					}

				} else if (bytes_read == 0) {															//	No data (usually means a client disconected)
					event->client->remove(); return (1);
				} else if (bytes_read == -1) {															//	Error reading
					Log::log(RED500 "Communication failed with " BLUE400 + event->client->ip + NC, Log::BOTH_ERROR, event->socket->VServ);
					event->client->remove(); return (1);
				}		

				return (0);
			}

		#pragma endregion

		#pragma region Write

			void Communication::write_client(EventInfo * event) {
				if (!event) return;

				if (!event->write_buffer.empty()) {														//	There are data in the 'write_buffer'

					event->client->update_last_activity();												//	Reset client timeout
					Event::update_last_activity(event->fd);												//	Reset event timeout

					size_t buffer_size = event->write_buffer.size();									//	Set the size of the chunk
					size_t chunk = CHUNK_SIZE;
					if (buffer_size > 0) chunk = std::min(buffer_size, static_cast<size_t>(CHUNK_SIZE));
					
					ssize_t bytes_written = send(event->fd, event->write_buffer.data(), chunk, MSG_DONTWAIT);					//	Send the data

					if (bytes_written > 0) {																					//	Some data is sent
						event->write_buffer.erase(event->write_buffer.begin(), event->write_buffer.begin() + bytes_written);	//	Remove the data sent from 'write_buffer'

						event->file_read += bytes_written;																		//	Increase file_read

						Thread::inc_size_t(Display::mutex, write_bytes, bytes_written);											//	Increase total bytes written
					} else if (bytes_written == 0) {																			//	No data, wait for more data or timeout
						return;
					} else if (bytes_written == -1) {																			//	Error writing
						event->client->remove(); return;
					}
				}

				if ((event->file_info == 0 && event->file_read >= event->file_size) || (event->file_info == 2 && event->write_buffer.empty())) {	//	All data has been sent
					long MaxRequests = Settings::KEEP_ALIVE_TIMEOUT;
					if (Settings::global.get("keepalive_requests") != "") Utils::stol(Settings::global.get("keepalive_requests"), MaxRequests);		//	Get the maximum request allowed

					std::string time = Utils::ltos(Settings::timer.elapsed_milliseconds(event->response_time));										//	Get the time to process the request in milliseconds
					gettimeofday(&event->response_time, NULL);																						//	Reset response time
					
					Log::log("TRF|GET|/|" + event->response_map["code"] + "|" + Utils::ltos(event->response_size) + "|" + time + "|" + event->client->ip, Log::BOTH_ACCESS, event->socket->VServ, event->vserver_data);	//	Log the client request

					if (event->header_map["Connection"] == "close" || event->client->total_requests + 1 >= MaxRequests)								//	Close the connection if client ask or max request reach
						event->client->remove();
					else {
						event->client->total_requests++;																							//	Increase total_requests
						Epoll::set(event->fd, true, false);																							//	Monitor read events only in EPOLL 
					}
				}

			}

		#pragma endregion

	#pragma endregion

	#pragma region DATA

		#pragma region Read

			int Communication::read_data(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));					//	Initialize buffer

				ssize_t bytes_read = read(event->fd, buffer, CHUNK_SIZE);								//	Read a chunk

				if (bytes_read > 0) {																	//	Read some data

					Event::update_last_activity(event->fd);												//	Reset event timeout

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);	//	Store the data read into 'read_buffer'
					event->file_read += bytes_read;														//	Increase file_read

					if (event->no_cache == true) {																							//	No cache allowed
						EventInfo * c_event = Event::get(event->client->fd);																//	Get the event of the client
						c_event->write_buffer.insert(c_event->write_buffer.end(), buffer, buffer + bytes_read);								//	Copy the data to the 'write_buffer' of the client
						if (event->file_read >= event->file_size) {																			//	All data has been read
							c_event->file_info = 2;																							//	Send a flag to the client that no more data is coming
							Event::remove(event->fd); return (1);																			//	Remove the event
						}

					} else if (event->file_read >= event->file_size) {																		//	Cache allowed and all data has been read
						std::string data(event->read_buffer.begin(), event->read_buffer.end());												//	Create a string with the data
						cache.add(event->file_path, data);																					//	Add the data to the cache

						EventInfo * c_event = Event::get(event->client->fd);																//	Get the event of the client
						c_event->file_info = 2;																								//	Send a flag to the client that no more data is coming
						c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());	//	Copy the data to the 'write_buffer' of the client
						Event::remove(event->fd); return (1);																				//	Remove the event
					}

				} else if (bytes_read == 0) {																														//	No data
					EventInfo * c_event = Event::get(event->client->fd);																							//	Get the event of the client
					c_event->file_info = 2;																															//	Send a flag to the client that no more data is coming
					if (event->no_cache == false) c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());	//	Copy the data to the 'write_buffer' of the client
					Event::remove(event->fd); return (1);																											//	Remove the event
				} else if (bytes_read == -1) {																														//	Error reading
					EventInfo * c_event = Event::get(event->client->fd);																							//	Get the event of the client
					c_event->file_info = 2;																															//	Send a flag to the client that no more data is coming
					if (event->no_cache == false) c_event->write_buffer.insert(c_event->write_buffer.end(), event->read_buffer.begin(), event->read_buffer.end());	//	Copy the data to the 'write_buffer' of the client
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

			int Communication::read_cgi(EventInfo * event) {
				if (!event) return (0);

				char buffer[CHUNK_SIZE];			memset(buffer, 0, sizeof(buffer));					//	Initialize buffer

				ssize_t bytes_read = read(event->fd, buffer, CHUNK_SIZE);								//	Read a chunk

				if (bytes_read > 0) {																	//	Read some data

					Event::update_last_activity(event->fd);												//	Reset event timeout

					EventInfo * c_event = Event::get(event->client->fd);								//	Get the event of the client

					event->read_buffer.insert(event->read_buffer.end(), buffer, buffer + bytes_read);	//	Store the data read into 'read_buffer'
					event->file_read += bytes_read;														//	Increase file_read

					if (event->file_info == 0 && event->file_size == 0) {												//	Needs to get 'content_length'
						if (event->header == "") {																		//	If no header yet, try to get one
							size_t content_length = 0;
							if (event->read_buffer.size() > HEADER_MAXSIZE) {											//	Header too big, return error
								Event::remove(event->fd); return (1);
							}
							int result = Protocol::parse_header(event);													//	Try to parse the header (maybe the header is not there yet)
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

					c_event->write_buffer.insert(c_event->write_buffer.end(), buffer, buffer + bytes_read);				//	Copy the data to the 'write_buffer' of the client

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

			void Communication::write_cgi(EventInfo * event) {
				if (!event) return;

				if (!event->write_buffer.empty()) {																									//	There are data in the 'write_buffer'

					Event::update_last_activity(event->fd);																							//	Reset event timeout

					size_t buffer_size = event->write_buffer.size();																				//	Set the size of the chunk
					size_t chunk = CHUNK_SIZE;
					if (buffer_size > 0) chunk = std::min(buffer_size, static_cast<size_t>(CHUNK_SIZE));
					
					ssize_t bytes_written = write(event->fd, event->write_buffer.data(), chunk);													//	Send the data

					if (bytes_written > 0) {																										//	Some data is sent
						event->write_buffer.erase(event->write_buffer.begin(), event->write_buffer.begin() + bytes_written);						//	Remove the data sent from 'write_buffer'

						event->file_read += bytes_written;																							//	Increase file_read

					} else { Event::remove(event->fd); return; }																					//	Error writing, remove the event
				}

				if ((event->file_info == 0 && event->file_read >= event->file_size) || (event->file_info == 2 && event->write_buffer.empty())) {	//	All data has been sent
					Event::remove(event->fd);																										//	Remove the event
				}
			}

		#pragma endregion

	#pragma endregion

#pragma endregion
