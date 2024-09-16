/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Net.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:55:43 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/16 17:19:40 by vzurera-         ###   ########.fr       */
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
	std::map <int, Net::EventInfo>			Net::events;												//	Map of all events objects
	Cache									Net::cache(600, 100, 10);									//	Used to store cached data, such as files or HTML responses.	(arguments: expiration in seconds, max entries, max content size in MB)

	int										Net::total_clients;											//	Total number of clients conected
	long									Net::read_bytes;											//	Total number of bytes downloaded by the server
	long									Net::write_bytes;											//	Total number of bytes uploaded by the server

	bool									Net::do_cleanup = false;
	int										Net::ask_socket = 0;										//	Flag indicating the request to create or close all sockets		(Used when Key_W is pressed)

	std::list<std::pair <VServer *, int> >	Net::socket_action_list;									//	List of VServers to enable or disable							(Used when Key_V is pressed)

	int 									Net::epoll_fd = -1;											//	File descriptor for epoll
	int										Net::timeout_fd = -1;										//	EventInfo structure used for generating events in epoll and checking client timeouts

	const int								Net::MAX_EVENTS = 10;										//	Maximum number of events that can be handled per iteration by epoll
	const size_t							Net::CHUNK_SIZE = 4096;										//	Size of the buffer for read and write operations
	const int								Net::TIMEOUT_INTERVAL = 1;									//	Interval in seconds between timeout checks for inactive clients
	
	const int								Net::KEEP_ALIVE_TIMEOUT = 2;								//	Timeout in seconds for keep-alive (if a client is inactive for this amount of time, the connection will be closed)
	const int								Net::KEEP_ALIVE_REQUEST = 500;								//	Maximum request for keep-alive (if a client exceeds this number of requests, the connection will be closed)

	enum e_socket_error { SK_CREATE, SK_CONFIGURE, SK_BIND, SK_LISTEN, SK_EPOLL };						//	Enumaration for socket errors

	#pragma region EventInfo

		#pragma region Constructors

			Net::EventInfo::EventInfo() : fd(-1), type(NOTHING), socket(NULL), client(NULL) {
				pipe[0] = -1; pipe[1] = -1; data_size = 0; max_data_size = 0; path = ""; no_cache = false; close = false;
			}

			Net::EventInfo::EventInfo(int _fd, int _type, Net::SocketInfo * _socket, Client * _client) : fd(_fd), type(_type), socket(_socket), client(_client) {
				pipe[0] = -1; pipe[1] = -1; data_size = 0; max_data_size = 0; path = ""; no_cache = false; close = false;
			}

			Net::EventInfo::EventInfo(const EventInfo & src) { *this = src; }

		#pragma endregion

		#pragma region Overloads

			Net::EventInfo & Net::EventInfo::operator=(const EventInfo & rhs) {
				if (this != &rhs) {
					fd = rhs.fd; type = rhs.type; socket = rhs.socket; client = rhs.client; path = rhs.path; no_cache = rhs.no_cache; close = rhs.close; request = rhs.request;
					pipe[0] = rhs.pipe[0]; pipe[1] = rhs.pipe[1]; data_size = rhs.data_size; max_data_size = rhs.max_data_size; read_buffer = rhs.read_buffer; write_buffer = rhs.write_buffer;
				}
				return (*this);
			}

			bool Net::EventInfo::operator==(const EventInfo & rhs) const {
				return (fd == rhs.fd);
			}

		#pragma endregion

	#pragma endregion

	#pragma region SocketInfo

		#pragma region Constructors

			Net::SocketInfo::SocketInfo(int _fd, const std::string & _IP, int _port, VServer * _VServ) : fd(_fd), IP(_IP), port(_port), VServ(_VServ) {}
			Net::SocketInfo::SocketInfo(const SocketInfo & src) { *this = src; }

		#pragma endregion

		#pragma region Overloads

			Net::SocketInfo & Net::SocketInfo::operator=(const SocketInfo & rhs) {
				if (this != &rhs) { fd = rhs.fd; IP = rhs.IP; port = rhs.port; VServ = rhs.VServ; clients = rhs.clients; }
				return (*this);
			}

			bool Net::SocketInfo::operator==(const SocketInfo & rhs) const {
				return (fd == rhs.fd && IP == rhs.IP && port == rhs.port && VServ == rhs.VServ && clients == rhs.clients);
			}

		#pragma endregion

		#pragma region Remove

			void Net::SocketInfo::remove() {
				Net::epoll_del(fd); close(fd); remove_event(fd);
				std::list <SocketInfo>::iterator s_it = Net::sockets.begin();
				while (s_it != Net::sockets.end()) {
					if (*s_it == *this) {
						std::list <Client *>::iterator c_it = clients.begin();
						while (c_it != clients.end()) {
							std::list <Client *>::iterator curr_c_it = c_it;
							Client * current = *c_it; ++c_it;
							current->remove(true);
							clients.erase(curr_c_it);
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

#pragma region Cleanup

	void Net::cleanup_socket() {
		if (!do_cleanup) return;
		// Iterar sobre cada socket en la lista de sockets
		std::list<SocketInfo>::iterator s_it = sockets.begin();
		while (s_it != sockets.end()) {
			// Iterar sobre cada cliente en la lista de clientes del socket
			std::list<Client *>::iterator c_it = s_it->clients.begin();
			while (c_it != s_it->clients.end()) {
				// Comprobar si el cliente está en la lista general de clientes
				bool found = false;
				std::list<Client>::iterator gc_it = clients.begin();
				while (gc_it != clients.end()) {
					if (*gc_it == **c_it) { found = true; break; }
					++gc_it;
				}
				// Si el cliente no está en la lista general, eliminarlo de la lista del socket
				if (!found)				c_it = s_it->clients.erase(c_it);
				else					++c_it;
			}
			++s_it;
		}
	}

#pragma endregion

#pragma region Sockets

	#pragma region Create

		#pragma region Create All

			int Net::socket_create_all() {
				bool nothing_created = true;

				if (Thread::get_bool(Display::mutex, Settings::global.status) == false) return (1);

				for (std::deque <VServer>::iterator vserv_it = Settings::vserver.begin(); vserv_it != Settings::vserver.end(); ++vserv_it)
					if (!socket_create(&(*vserv_it))) nothing_created = false;

				return (nothing_created);
			}

		#pragma endregion

		#pragma region Create VServer

			#pragma region Error Messages

				static void socket_error(std::vector <std::pair<std::string, int> >::const_iterator addr_it, VServer * VServ, int type) {
					return;
					switch (type) {
						case SK_CREATE:
							Log::log("Socket " + addr_it->first + ":" + Utils::ltos(addr_it->second) + " cannot be created", Log::MEM_ERROR, VServ); break;
						case SK_CONFIGURE:
							Log::log("Socket " + addr_it->first + ":" + Utils::ltos(addr_it->second) + " cannot be configured", Log::MEM_ERROR, VServ); break;
						case SK_BIND:
							Log::log("Socket " + addr_it->first + ":" + Utils::ltos(addr_it->second) + " cannot be binded", Log::MEM_ERROR, VServ); break;
						case SK_LISTEN:
							Log::log("Socket " + addr_it->first + ":" + Utils::ltos(addr_it->second) + " cannot listen", Log::MEM_ERROR, VServ); break;
						case SK_EPOLL:
							Log::log("Socket " + addr_it->first + ":" + Utils::ltos(addr_it->second) + " cannot be added to EPOLL", Log::MEM_ERROR, VServ); break;
					}
				}

			#pragma endregion

			int Net::socket_create(VServer * VServ) {
				//	If the server or the virtual server is disabled do nothing
				bool nothing_created = true;
				if (Thread::get_bool(Display::mutex, Settings::global.status) == false || VServ->bad_config || Thread::get_bool(Display::mutex, VServ->force_off)) return (1);

				//	For every IP address in the virtual server create a socket
				for (std::vector <std::pair<std::string, int> >::const_iterator addr_it = VServ->addresses.begin(); addr_it != VServ->addresses.end(); ++addr_it) {
					if (socket_exists(addr_it->first, addr_it->second)) continue;

					//	Create socket
					int fd = socket(AF_INET, SOCK_STREAM, 0);
					if (fd == -1) { socket_error(addr_it, VServ, SK_CREATE); continue; }

					//	Configure socket
					int options = 1;
					if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options)) == -1) { socket_error(addr_it, VServ, SK_CONFIGURE); close(fd); continue; }

					//	Initialize the socket address structure with the IP address and port
					sockaddr_in address; std::memset(&address, 0, sizeof(address));
					address.sin_family = AF_INET;
					address.sin_port = htons(addr_it->second);
					if (addr_it->first == "0.0.0.0")	address.sin_addr.s_addr = INADDR_ANY;
					else 								inet_pton(AF_INET, addr_it->first.c_str(), &address.sin_addr);

					//	Link the address to the socket (0.0.0.0 links to all available network interfaces)
					if (bind(fd, (sockaddr *)&address, sizeof(address)) == -1) { socket_error(addr_it, VServ, SK_BIND); close(fd); continue; }

					//	Listen on the address for incoming connections
					if (listen(fd, SOMAXCONN) == -1) { socket_error(addr_it, VServ, SK_LISTEN); close(fd); continue; }

					//	Add the socket FD to EPOLL
					if (epoll_add(fd, true, false) == -1) { socket_error(addr_it, VServ,SK_EPOLL); close(fd); continue; }

					// Create a SocketInfo, an EventInfo and add them to Net::sockets and Net::events
					sockets.push_back(SocketInfo(fd, addr_it->first, addr_it->second, VServ));
					events[fd] = EventInfo(fd, SOCKET, &sockets.back(), NULL);

					//	Set the virtual server as active (this means it has sockets associated with it)
					if (Thread::get_bool(Display::mutex, VServ->status) == false) Thread::set_bool(Display::mutex, VServ->status, true);

					//	Log message
					std::string ip = addr_it->first;
					if (ip == "0.0.0.0") ip = "All interfaces";
					std::string port = Utils::ltos(addr_it->second);
					std::string msg = UN BLUE400 + ip + NC + std::string("                ").substr(ip.size()) + C " listening on port " + UN BLUE400 + port + NC;
					Log::log(msg, Log::MEM_ACCESS, VServ);

					nothing_created = false;
				}
				if (!nothing_created) Log::log("---", Log::MEM_ACCESS, VServ);
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
				Thread::set_int(Display::mutex, total_clients, 0);
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

			clients.push_back(Client(fd, event->socket, IP, port));
			events[fd] = EventInfo(fd, CLIENT, event->socket, NULL);
			events[fd].client = &clients.back();

			if (epoll_add(fd, true, false) == -1) { events[fd].client->remove(); return; }

			Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
			total_clients++;
			Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);
		}

	#pragma endregion

	#pragma region Status

		int Net::server_status() {
			bool update_display = false;
			bool recreate_sockets = false;

			Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

			if (socket_action_list.size() > 0) {
				std::list <std::pair<VServer *, int> >::iterator it = socket_action_list.begin();
				while (it != socket_action_list.end()) {

					Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

						if (it->second == CREATE) socket_create(it->first);
						if (it->second == CLOSE) { socket_close(it->first); recreate_sockets = true; }
						
					Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

					it = socket_action_list.erase(it);
				}
				update_display = true;
			}

			Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);
	
			if (recreate_sockets) socket_create_all();
			if (update_display) Display::update();

			Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

				if (ask_socket == 1) { ask_socket = 0; Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK); socket_create_all(); Display::update(); return (1); }
				if (ask_socket == 2) { ask_socket = 0; Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK); socket_close_all(); Display::update(); return (2); }

			Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region Events

	#pragma region Get

		Net::EventInfo * Net::get_event(int fd) {
			if (fd < 0) return (NULL);
			std::map<int, EventInfo>::iterator it = events.find(fd);

			if (it != events.end())		return (&it->second);
			else 						return (NULL);
		}

	#pragma endregion

	#pragma region Remove

		#pragma region Remove (One)

			int Net::remove_event(int fd) {
				EventInfo * event = get_event(fd);
				if (!event) return (1);

				if (event->type == DATA) {
					epoll_del(event->fd);
					if (event->fd != -1) close(event->fd);
					if (event->pipe[0] != -1) close(event->pipe[0]);
					if (event->pipe[1] != -1) close(event->pipe[1]);
				}
				events.erase(event->fd);

				return (0);
			}

		#pragma endregion

		#pragma region Remove (Client)

			void Net::remove_events(Client * Cli) {
				if (events.size() == 0) return;
				std::map<int, EventInfo>::iterator it = events.begin();
				while (it != events.end()) {
					std::map<int, EventInfo>::iterator current = it++;
					if (current->second.client == Cli) {
						epoll_del(current->second.fd);
						if (current->second.fd != -1) close(current->second.fd);
						if (current->second.type == DATA) {
							if (current->second.pipe[0] != -1) close(current->second.pipe[0]);
							if (current->second.pipe[1] != -1) close(current->second.pipe[1]);
						}
						events.erase(current);
					}
				}
			}

		#pragma endregion

		#pragma region Remove (All)

			void Net::remove_events() {
				if (events.size() == 0) return;
				std::map<int, EventInfo>::iterator it = events.begin();
				while (it != events.end()) {
					std::map<int, EventInfo>::iterator current = it++;
					epoll_del(current->second.fd);
					if (current->second.fd != -1) close(current->second.fd);
					if (current->second.type == DATA) {
						if (current->second.pipe[0] != -1) close(current->second.pipe[0]);
						if (current->second.pipe[1] != -1) close(current->second.pipe[1]);
					}
					events.erase(current);
				}
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region EPOLL

	#pragma region Time-Out

		#pragma region Create

			int Net::create_timeout() {
				timeout_fd = timerfd_create(CLOCK_MONOTONIC, 0);
				if (timeout_fd == -1) return (1);

				struct itimerspec new_value;
				memset(&new_value, 0, sizeof(new_value));
				new_value.it_value.tv_sec = Net::TIMEOUT_INTERVAL;		// Tiempo hasta la primera expiración
				new_value.it_interval.tv_sec = Net::TIMEOUT_INTERVAL; 	// Intervalo entre expiraciones

				if (timerfd_settime(timeout_fd, 0, &new_value, NULL) == -1) { close(timeout_fd); timeout_fd = -1; return (1); }

				return (0);
			}

		#pragma endregion

		#pragma region Check

			void Net::check_timeout() {
				uint64_t expirations;
				read(timeout_fd, &expirations, sizeof(expirations));

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

	#pragma region Create

		int Net::epoll__create() {
				if (epoll_fd != -1) epoll_close();

				epoll_fd = epoll_create(1024);
				if (epoll_fd == -1) { Log::log(RD "Cannot create " Y "EPOLL" NC, Log::MEM_ERROR); return (1); }
				if (!create_timeout()) epoll_add(timeout_fd, true, false);

				return (0);
		}

	#pragma endregion

	#pragma region Close

		void Net::epoll_close() {
			if (timeout_fd != -1) { close(timeout_fd); timeout_fd = -1; }
			if (epoll_fd != -1) { close(epoll_fd); epoll_fd = -1; }
		}

	#pragma endregion
	
	#pragma region Add

		int Net::epoll_add(int fd, bool epollin, bool epollout) {
			if (fd < 0) return (-1);
			struct epoll_event epoll_event;

			epoll_event.data.fd = fd;

			if (epollin && epollout)	epoll_event.events = EPOLLIN | EPOLLOUT;
			else if (epollin) 			epoll_event.events = EPOLLIN;
			else if (epollout) 			epoll_event.events = EPOLLOUT;
			else { 						return (-1); }

			return (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event));
		}

	#pragma endregion

	#pragma region Set

		int Net::epoll_set(int fd, bool epollin, bool epollout) {
			if (fd < 0) return (-1);
			struct epoll_event epoll_event;

			epoll_event.data.fd = fd;

			if (epollin && epollout)	epoll_event.events = EPOLLIN | EPOLLOUT;
			else if (epollin) 			epoll_event.events = EPOLLIN;
			else if (epollout)			epoll_event.events = EPOLLOUT;
			else {				 		return (0); }

			return (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &epoll_event));
		}

	#pragma endregion

	#pragma region Del

		void Net::epoll_del(int fd) {
			if (epoll_fd < 0 || fd < 0) return;
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		}

	#pragma endregion

	#pragma region Events

		int Net::epoll_events() {
			if (epoll_fd < 0) return (2);
			struct epoll_event events[MAX_EVENTS];

			int eventCount = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);
			if (eventCount == -1) return (1);

			for (int i = 0; i < eventCount; ++i) {
				if (events[i].data.fd == timeout_fd) { Display::update(); check_timeout(); cache.remove_expired(); continue; }

				EventInfo * event = get_event(events[i].data.fd);
				if (!event) continue;

				if (events[i].events & EPOLLIN) {
					switch (event->type) {
						case SOCKET: 	{ socket_accept(event);		break; }
						case DATA: 		{ read_data(event);			break; }
						case CGI: 		{ read_data(event);			break; }
						case CLIENT: 	{ read_client(event);		break; }
					}
				}

				event = get_event(events[i].data.fd);
				if (!event) continue;

				if (events[i].events & EPOLLOUT) {
					switch (event->type) {
						case CLIENT: 	{ write_client(event);		break; }
						case DATA: 		{							break; }
						case CGI: 		{							break; }
					}
				}
			}

			server_status();
			cleanup_socket();

			return (0);
		}

	#pragma endregion

#pragma endregion
