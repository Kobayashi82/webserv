/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Net.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:55:43 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/25 11:18:32 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Colors.hpp"
#include "Net.hpp"
#include "Client.hpp"
#include "Display.hpp"
#include "Thread.hpp"

#pragma region Variables

	std::list<Net::SocketInfo>				Net::sockets;												//	List of all SocketInfo objects
	std::list<Client>						Net::clients;												//	List of all Client objects
	Cache									Net::cache(600, 100, 10);									//	Used to store cached data, such as files or HTML responses.	(arguments: expiration in seconds, max entries, max content size in MB)

	int										Net::total_clients;											//	Total number of clients conected
	long									Net::read_bytes;											//	Total number of bytes downloaded by the server
	long									Net::write_bytes;											//	Total number of bytes uploaded by the server

	bool									Net::ask_socket_create_all;									//	Flag indicating the request to create all sockets			(Used when Key_W is pressed)
	bool									Net::ask_socket_close_all;									//	Flag indicating the request to close of all sockets			(Used when Key_W is pressed)
	std::list<std::pair <VServer *, int> >	Net::socket_action_list;									//	List of VServers to enable or disable						(Used when Key_V is pressed)

	int 									Net::epoll_fd = -1;											//	File descriptor for epoll
	Net::EventInfo							Net::event_timeout;											//	EventInfo structure used for generating events in epoll and checking client timeouts

	const int								Net::MAX_EVENTS = 10;										//	Maximum number of events that can be handled per iteration by epoll
	const int								Net::EPOLL_BUFFER_SIZE = 4096;								//	Size of the buffer for read and write operations
	const int								Net::TIMEOUT_INTERVAL = 1;									//	Interval in seconds between timeout checks for inactive clients
	
	const int								Net::KEEP_ALIVE_TIMEOUT = 30;								//	Timeout in seconds for keep-alive (if a client is inactive for this amount of time, the connection will be closed)
	const int								Net::KEEP_ALIVE_REQUEST = 500;								//	Maximum request for keep-alive (if a client exceeds this number of requests, the connection will be closed)

	#pragma region EventInfo

		#pragma region Constructors

			Net::EventInfo::EventInfo() : fd(-1) {}
			Net::EventInfo::EventInfo(int _fd, int _type, Net::SocketInfo * _socket, Client * _client) : fd(_fd), type(_type), socket(_socket), client(_client) {}

		#pragma endregion

		#pragma region Overloads

			Net::EventInfo & Net::EventInfo::operator=(const EventInfo & rhs) {
				if (this != &rhs) { fd = rhs.fd; type = rhs.type; socket = rhs.socket; client = rhs.client; }
				return (*this);
			}

			bool Net::EventInfo::operator==(const EventInfo & rhs) {
				return (fd == rhs.fd && type == rhs.type && socket == rhs.socket && client == rhs.client);
			}

		#pragma endregion

	#pragma endregion

	#pragma region SocketInfo

		#pragma region Constructors

			Net::SocketInfo::SocketInfo(int _fd, const std::string & _IP, int _port, EventInfo _event, VServer * _VServ) : fd(_fd), IP(_IP), port(_port), event(_event), VServ(_VServ) {}

		#pragma endregion

		#pragma region Overloads

			Net::SocketInfo & Net::SocketInfo::operator=(const SocketInfo & rhs) {
				if (this != &rhs) { fd = rhs.fd; IP = rhs.IP; port = rhs.port; event = rhs.event; VServ = rhs.VServ; clients = rhs.clients; }
				return (*this);
			}

			bool Net::SocketInfo::operator==(const SocketInfo & rhs) {
				return (fd == rhs.fd && IP == rhs.IP && port == rhs.port && event == rhs.event && VServ == rhs.VServ && clients == rhs.clients);
			}

		#pragma endregion

		#pragma region Remove

			void Net::SocketInfo::remove() {
				Net::epoll_del(&event); close(fd);
				std::list <SocketInfo>::iterator s_it = Net::sockets.begin();
				while (s_it != Net::sockets.end()) {
					if (*s_it == *this) {
						std::list <Client *>::iterator c_it = clients.begin();
						while (c_it != clients.end()) {
							Client * current = *c_it; ++c_it;
							current->remove();
						}
						Net::sockets.erase(s_it);
						break;
					}
					++s_it;
				}
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Sockets

	#pragma region Create

		#pragma region Create All

			int Net::socket_create_all() {
				if (Thread::get_bool(Display::mutex, Settings::global.status) == false) return (1);

				bool nothing_created = true;

				for (std::deque <VServer>::iterator vserv_it = Settings::vserver.begin(); vserv_it != Settings::vserver.end(); ++vserv_it)
					if (!socket_create(&(*vserv_it))) nothing_created = false;

				return (nothing_created);
			}

		#pragma endregion

		#pragma region Create VServer

			int Net::socket_create(VServer * VServ) {
				if (Thread::get_bool(Display::mutex, Settings::global.status) == false || Thread::get_bool(Display::mutex, VServ->force_off)) return (1);

				bool nothing_created = true;

				for (std::vector <std::pair<std::string, int> >::const_iterator addr_it = VServ->addresses.begin(); addr_it != VServ->addresses.end(); ++addr_it) {
					if (socket_exists(addr_it->first, addr_it->second)) continue;

					int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
					if (serverSocket == -1) {
						Log::log("Socket " + addr_it->first + ":" + Utils::ltos(addr_it->second) + " cannot be created", Log::BOTH_ERROR, VServ);
						continue;
					}

					int options = 1;
					if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options)) == -1) {
						Log::log("Socket " + addr_it->first + ":" + Utils::ltos(addr_it->second) + " cannot be created", Log::BOTH_ERROR, VServ);
						close(serverSocket); continue;
					}

					sockaddr_in address; std::memset(&address, 0, sizeof(address));
					address.sin_family = AF_INET;
					address.sin_port = htons(addr_it->second);
					inet_pton(AF_INET, addr_it->first.c_str(), &address.sin_addr);

					if (bind(serverSocket, (sockaddr *)&address, sizeof(address)) == -1) {
						//Log::log(Utils::ltos(errno) + "Error vinculando el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second));
						close(serverSocket); continue;
					}

					if (listen(serverSocket, SOMAXCONN) == -1) {
						Log::log("Socket " + addr_it->first + ":" + Utils::ltos(addr_it->second) + " cannot be created", Log::BOTH_ERROR, VServ);
						close(serverSocket); continue;
					}

					// Add the new socket as SocketInfo to Net::sockets
					sockets.push_back(SocketInfo(serverSocket, addr_it->first, addr_it->second, EventInfo(serverSocket, SOCKET, NULL, NULL), VServ));
					SocketInfo &socketInfo = sockets.back();
					socketInfo.event.socket = &socketInfo;

					if (epoll_add(&(socketInfo.event)) == -1) {
						Log::log("Socket " + addr_it->first + ":" + Utils::ltos(addr_it->second) + " cannot be created", Log::BOTH_ERROR, VServ);
						close(serverSocket); sockets.pop_back(); continue;
					}

					if (Thread::get_bool(Display::mutex, VServ->status) == false) Thread::set_bool(Display::mutex, VServ->status, true);
					//Log::log("Socket " + addr_it->first + ":" + Utils::ltos(addr_it->second) + " waiting for connections", Log::BOTH_ACCESS, VServ);
					nothing_created = false;
				}

				return (nothing_created);
			}

		#pragma endregion	

		#pragma region Exists

			bool Net::socket_exists(const std::string & IP, int port) {
				for (std::list <SocketInfo>::const_iterator it = sockets.begin(); it != sockets.end(); ++it)
					if (it->IP == IP && it->port == port) return (true);
				return (false);
			}

		#pragma endregion

	#pragma endregion

	#pragma region Close

		#pragma region Close All

			void Net::socket_close_all() {
				std::list<SocketInfo>::iterator s_it = sockets.begin();
				while (s_it != sockets.end()) {
					if (Thread::get_bool(Display::mutex, s_it->VServ->status)) Thread::set_bool(Display::mutex, s_it->VServ->status, false);
					SocketInfo current = *s_it; ++s_it;
					current.remove();
				}
			}

		#pragma endregion

		#pragma region Close VServer

			void Net::socket_close(VServer * VServ) {
				std::list<SocketInfo>::iterator s_it = sockets.begin();
				while (s_it != sockets.end()) {
					if (s_it->VServ == VServ) {
						SocketInfo current = *s_it; ++s_it;
						current.remove();
					} else ++s_it;
				}
				Thread::set_bool(Display::mutex, VServ->status, false);
			}

		#pragma endregion

	#pragma endregion

	#pragma region Accept

		void Net::socket_accept(EventInfo * event) {
			sockaddr_in Addr; socklen_t AddrLen = sizeof(Addr);
			int fd = accept(event->fd, (sockaddr *)&Addr, &AddrLen); if (fd == -1) return;

			std::string	IP		= inet_ntoa(Addr.sin_addr);
			int			port	= ntohs(Addr.sin_port);

			clients.push_back(Client(fd, event->socket, IP, port, EventInfo(fd, CLIENT, event->socket, NULL)));
			Client & cli = clients.back();
			event->socket->clients.push_back(&cli);
			cli.event.client = &cli;

			if (epoll_add(&(cli.event)) == -1) { close(fd); clients.pop_back(); event->socket->clients.pop_back(); return; }

			Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
			total_clients++;
			Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);
		}

	#pragma endregion

#pragma endregion

#pragma region EPOLL

	#pragma region Create

		int Net::epoll__create() {
				if (epoll_fd != -1) epoll_close();

				epoll_fd = epoll_create(1024);
				if (epoll_fd == -1) { Log::log(RD "Cannot create " Y "EPOLL" NC, Log::BOTH_ERROR); return (1); }
				if (!create_timeout()) epoll_add(&event_timeout);

				return (0);
		}

	#pragma endregion

	#pragma region Close

		void Net::epoll_close() {
			if (event_timeout.fd != -1) close(event_timeout.fd);
			if (epoll_fd != -1) close(epoll_fd);
		}

	#pragma endregion
	
	#pragma region Add

		int Net::epoll_add(EventInfo * event) {
			struct epoll_event epoll_event;

			epoll_event.data.ptr = event;
			epoll_event.events = EPOLLIN;
		
			return (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event->fd, &epoll_event));
		}

	#pragma endregion

	#pragma region Edit

		int Net::epoll_edit(EventInfo * event, bool epollin, bool epollout) {
			struct epoll_event epoll_event;

			epoll_event.data.ptr = event;

			if (epollin && epollout) epoll_event.events = EPOLLIN | EPOLLOUT;
			else if (epollin) epoll_event.events = EPOLLIN;
			else if (epollout) epoll_event.events = EPOLLOUT;
			else return (0);
		
			return (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event->fd, &epoll_event));
		}

	#pragma endregion

	#pragma region Del

		void Net::epoll_del(EventInfo * event) {
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event->fd, NULL);
		}

	#pragma endregion

	#pragma region Events

		int Net::epoll_events() {
			bool update_display = false;
			struct epoll_event events[MAX_EVENTS];
			int eventCount = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);
			if (eventCount == -1) return (1);

			for (int i = 0; i < eventCount; ++i) {
				if (events[i].data.ptr == NULL) continue;

				EventInfo * event = static_cast<EventInfo *>(events[i].data.ptr);

				if (events[i].events & EPOLLIN) {
					switch (event->type) {
						case SOCKET: 	{ socket_accept(event); break; }
						case CLIENT: 	{ if (read_request(event)) continue; break; }
						case DATA: 		{ if (read_data(event)) continue; break; }
						case CGI: 		{ if (read_data(event)) continue; break; }
						case TIMEOUT: 	{ Display::update(); check_timeout(); break; }
					}
				}
				if (events[i].events & EPOLLOUT) {
					switch (event->type) {
						case CLIENT: 	{ write_response(event); break; }
						case DATA: 		{ break; }
						case CGI: 		{ break; }
					}
				}
			}

			Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

				if (socket_action_list.size() > 0) {
					std::list <std::pair<VServer *, int> >::iterator it = socket_action_list.begin();
					while (it != socket_action_list.end()) {

						Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

							if (it->second == CREATE) socket_create(it->first);
							if (it->second == CLOSE) socket_close(it->first);
						
						Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

						it = socket_action_list.erase(it);
					}
					update_display = true;
				}

				Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);
				if (update_display) Display::update();

			Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

				if (ask_socket_create_all) { Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK); socket_create_all(); Display::update(); return (0); }
				if (ask_socket_close_all) { Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK); socket_close_all(); Display::update(); return (0); }

			Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

			return (0);
		}

	#pragma endregion

	#pragma region Time-Out

		#pragma region Create

			int Net::create_timeout() {
				int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
				if (timer_fd == -1) return (1);

				struct itimerspec new_value;
				memset(&new_value, 0, sizeof(new_value));
				new_value.it_value.tv_sec = Net::TIMEOUT_INTERVAL;		// Tiempo hasta la primera expiración
				new_value.it_interval.tv_sec = Net::TIMEOUT_INTERVAL; 	// Intervalo entre expiraciones

				if (timerfd_settime(timer_fd, 0, &new_value, NULL) == -1) { close(timer_fd); return (1); }

				event_timeout = EventInfo(timer_fd, TIMEOUT, NULL, NULL);

				return (0);
			}

		#pragma endregion

		#pragma region Check

			void Net::check_timeout() {
				uint64_t expirations;
				read(event_timeout.fd, &expirations, sizeof(expirations));

				long TimeOut = KEEP_ALIVE_TIMEOUT;

				if (Settings::global.get("keepalive_timeout") != "") Utils::stol(Settings::global.get("keepalive_timeout"), TimeOut);
				if (TimeOut == 0) return;

				std::list<Client>::iterator it = clients.begin();
				while (it != clients.end()) {
					std::list<Client>::iterator current = it; ++it;
					current->check_timeout(TimeOut);
				}
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Comunications

	#pragma region CLIENT

		#pragma region Read Request

			int Net::read_request(EventInfo * event) {
				char buffer[EPOLL_BUFFER_SIZE];				memset(buffer, 0, sizeof(buffer));
				char peek_buffer[EPOLL_BUFFER_SIZE + 1];	memset(peek_buffer, 0, sizeof(peek_buffer));

				ssize_t bytes_peek = recv(event->fd, peek_buffer, EPOLL_BUFFER_SIZE + 1, MSG_PEEK);

				if (bytes_peek <= 0) { event->client->remove(); return (1); }

				event->client->update_last_activity();

				ssize_t bytes_read = recv(event->fd, buffer, EPOLL_BUFFER_SIZE, 0);
				
				if (bytes_read > 0) {
					event->client->read_buffer.insert(event->client->read_buffer.end(), buffer, buffer + bytes_read);

					Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
					read_bytes+= bytes_read;
					Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

					//	if (event->client->read_buffer.size() >= max_request_size) return lo que sea

					if (bytes_peek <= EPOLL_BUFFER_SIZE) process_request(event);
				} else if (bytes_read <= 0) { event->client->remove(); return (1); }

				return (0);
			}

		#pragma endregion

		#pragma region Write Response

			void Net::write_response(EventInfo *event) {
				event->client->update_last_activity();

				if (!event->client->write_buffer.empty()) {

					size_t buffer_size = event->client->write_buffer.size();
					size_t chunk_size = std::min(buffer_size, static_cast<size_t>(EPOLL_BUFFER_SIZE));
					
					ssize_t bytes_written = write(event->fd, event->client->write_buffer.data(), chunk_size);

					if (bytes_written > 0) {
						event->client->write_buffer.erase(event->client->write_buffer.begin(), event->client->write_buffer.begin() + bytes_written);

						Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
						write_bytes+= bytes_written;
						Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

					} else if (bytes_written < 0) return; // close(event->fd);
				}
				
				if (event->client->write_buffer.empty()) {
					epoll_edit(event, true, false);

					long MaxRequests = KEEP_ALIVE_TIMEOUT;

					if (Settings::global.get("keepalive_requests") != "") Utils::stol(Settings::global.get("keepalive_requests"), MaxRequests);
					if (event->client->total_requests >= MaxRequests) event->client->remove();
				}
			}

		#pragma endregion

		#pragma region Process Request

			void Net::process_request(EventInfo * event) {

				//	Diferenciar entre dos request unidas. Se busca un header y eso determina el inicio de otro response?
				//	Si tenemos body_length leemos solo hasta ahi y el resto va de vuelta al buffer

				std::string request(event->client->read_buffer.begin(), event->client->read_buffer.end());

				//	if (body > max_body_size) return lo que sea

				std::istringstream request_stream(request);
				std::string line;
				if (std::getline(request_stream, line))
				Utils::trim(line);
				Log::log(line, Log::BOTH_ACCESS);

				event->client->read_buffer.clear();
				event->client->total_requests++;
				process_response(event);
			}

		#pragma endregion

		#pragma region Process Response

			void Net::process_response(EventInfo * event) {
				// Crear una respuesta HTTP básica
				std::string response = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"Content-Length: 48\r\n"
					"Connection: keep-alive\r\n"
					//"Connection: close\r\n"
					"\r\n"
					"<html><body><h1>Hello, World!</h1></body></html>";
				event->client->write_buffer.insert(event->client->write_buffer.end(), response.begin(), response.end());
				epoll_edit(event, true, true);

				//	Despues de procesar y enviar la respuesta, si total_request = max_requests cerrar cliente
			}

		#pragma endregion

	#pragma endregion

	#pragma region DATA

		#pragma region Read Data

			int Net::read_data(EventInfo * event) {
				char buffer[EPOLL_BUFFER_SIZE];				memset(buffer, 0, sizeof(buffer));
				char peek_buffer[EPOLL_BUFFER_SIZE + 1];	memset(peek_buffer, 0, sizeof(peek_buffer));

				ssize_t bytes_peek = recv(event->fd, peek_buffer, EPOLL_BUFFER_SIZE + 1, MSG_PEEK);

				// if (bytes_peek <= 0) { event->client->remove(); return (1); }										remove data from client->data

				ssize_t bytes_read = recv(event->fd, buffer, EPOLL_BUFFER_SIZE, 0);
				
				if (bytes_read > 0) {
					event->read_buffer.insert(event->client->read_buffer.end(), buffer, buffer + bytes_read);

					if (bytes_peek <= EPOLL_BUFFER_SIZE) process_data(event);
				} // else if (bytes_read <= 0) { event->client->remove(); return (1); }									remove data from client->data

				return (0);
			}

		#pragma endregion

		#pragma region Process Data

			void Net::process_data(EventInfo * event) {
				std::string request(event->read_buffer.begin(), event->client->read_buffer.end());

				//	if (body > max_body_size) return lo que sea

				std::istringstream request_stream(request);
				std::string line;
				if (std::getline(request_stream, line))
				Utils::trim(line);
				Log::log(line, Log::BOTH_ACCESS);

				event->read_buffer.clear();																	//	Aqui si vaciamos el buffer
				// process_response(event);																	process file to generate response
			}

		#pragma endregion

	#pragma endregion

#pragma endregion
