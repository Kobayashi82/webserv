/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sockets.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:55:43 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/18 21:01:15 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Sockets.hpp"
#include "Client.hpp"

#pragma region Variables

	std::list <Sockets::SocketInfo>		Sockets::sockets;
	std::list <Client>					Sockets::clients;
	int 								Sockets::epoll_fd = -1;

#pragma endregion

#pragma region Constructors

	Sockets::SocketInfo::SocketInfo(int _fd, const std::string & _IP, int _port, EventInfo _event, VServer * _VServ) : fd(_fd), IP(_IP), port(_port), event(_event), VServ(_VServ) {}
	Sockets::EventInfo::EventInfo(int _fd, int _type, Sockets::SocketInfo * _Socket, Client * _client) : fd(_fd), type(_type), Socket(_Socket), client(_client) {}

	Sockets::SocketInfo & Sockets::SocketInfo::operator=(const SocketInfo & rhs) {
		if (this != &rhs) {
			fd = rhs.fd;
			IP = rhs.IP;
			port = rhs.port;
			event = rhs.event;
			VServ = rhs.VServ;
			clients = rhs.clients;
		}
		return *this;
	}
	
	Sockets::EventInfo & Sockets::EventInfo::operator=(const EventInfo & rhs) {
		if (this != &rhs) {
			fd = rhs.fd;
			type = rhs.type;
			Socket = rhs.Socket;
			client = rhs.client;
		}
		return *this;
	}

#pragma endregion

#pragma region Sockets

	#pragma region Create

		#pragma region Create All

			int Sockets::socketCreate() {
				int not_created = 1;
				if (!Settings::global.status) return (1);
				for (std::deque <VServer>::iterator vserv_it = Settings::vserver.begin(); vserv_it != Settings::vserver.end(); ++vserv_it) {
					if (vserv_it->force_off) continue;
					for (std::vector <std::pair<std::string, int> >::const_iterator addr_it = vserv_it->addresses.begin(); addr_it != vserv_it->addresses.end(); ++addr_it) {
						if (socketExists(addr_it->first, addr_it->second)) { continue; }

						int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
						if (serverSocket == -1) { Log::log_error("Error creando el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second), &(*vserv_it)); continue; }

						int opt = 1;
						if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) { Log::log_error("Error configurando opciones de socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second), &(*vserv_it)); close(serverSocket); continue; }

						sockaddr_in address;
						std::memset(&address, 0, sizeof(address));
						address.sin_family = AF_INET;
						address.sin_port = htons(addr_it->second);
						inet_pton(AF_INET, addr_it->first.c_str(), &address.sin_addr);

						if (bind(serverSocket, (sockaddr*)&address, sizeof(address)) == -1) {
							//Log::log_error(Utils::ltos(errno) + "Error vinculando el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second));
							close(serverSocket); continue;
						}

						if (listen(serverSocket, SOMAXCONN) == -1) { Log::log_error("Error escuchando en el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second), &(*vserv_it)); close(serverSocket); continue; }

						// Añadir el socket al epoll
						sockets.push_back(SocketInfo(serverSocket, addr_it->first, addr_it->second, EventInfo(serverSocket, SOCKET, NULL, NULL), &(*vserv_it)));
						// Obtener una referencia al último elemento recién insertado.
						SocketInfo & socketInfo = sockets.back();

						// Establecer el puntero en el event al objeto SocketInfo que acabamos de insertar.
						socketInfo.event.Socket = &socketInfo;

						if (epoll_add(serverSocket, &socketInfo.event) == -1) { Log::log_error("Error añadiendo socket al epoll", &(*vserv_it)); close(serverSocket); sockets.pop_back(); continue; }

						if (!vserv_it->status) vserv_it->status = true;
						Log::log_access("Socket creado y escuchando en " + addr_it->first + ":" + Utils::ltos(addr_it->second), &(*vserv_it));
						not_created = 0;
					}
				}
				return (not_created);
			}

		#pragma endregion

		#pragma region Create One

			int Sockets::socketCreate(VServer * VServ) {
				if (!Settings::global.status || VServ->force_off) return (1);
				for (std::vector <std::pair<std::string, int> >::const_iterator addr_it = VServ->addresses.begin(); addr_it != VServ->addresses.end(); ++addr_it) {
					if (socketExists(addr_it->first, addr_it->second)) { continue; }

					int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
					if (serverSocket == -1) { Log::log_error("Error creando el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second), VServ); continue; }

					int opt = 1;
					if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) { Log::log_error("Error configurando opciones de socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second), VServ); close(serverSocket); continue; }

					sockaddr_in address; std::memset(&address, 0, sizeof(address));
					address.sin_family = AF_INET;
					address.sin_port = htons(addr_it->second);
					inet_pton(AF_INET, addr_it->first.c_str(), &address.sin_addr);

					if (bind(serverSocket, (sockaddr *)&address, sizeof(address)) == -1) {
						//Log::log_error(Utils::ltos(errno) + "Error vinculando el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second));
						close(serverSocket); continue;
					}

					if (listen(serverSocket, SOMAXCONN) == -1) { Log::log_error("Error escuchando en el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second), VServ); close(serverSocket); continue; }


					// Añadir el socket al epoll
					sockets.push_back(SocketInfo(serverSocket, addr_it->first, addr_it->second, EventInfo(serverSocket, SOCKET, NULL, NULL), VServ));
					// Obtener una referencia al último elemento recién insertado.
					SocketInfo &socketInfo = sockets.back();

					// Establecer el puntero en el event al objeto SocketInfo que acabamos de insertar.
					socketInfo.event.Socket = &socketInfo;

					if (epoll_add(serverSocket, &(socketInfo.event)) == -1) {
						Log::log_error("Error añadiendo socket al epoll", VServ); close(serverSocket); sockets.pop_back();
						continue;
					}

					if (!VServ->status) VServ->status = true;
					Log::log_access("Socket creado y escuchando en " + addr_it->first + ":" + Utils::ltos(addr_it->second), VServ);
				}
			return (true);
		}

		#pragma endregion

	#pragma endregion

	#pragma region Close

		#pragma region Close All

			void Sockets::socketClose() {
				std::list <SocketInfo>::iterator it = sockets.begin();
				while (it != sockets.end()) {
					if (it->VServ->status) it->VServ->status = false;
					socketClose(&(*it), false);
					it = sockets.erase(it);
				}
			}

		#pragma endregion

		#pragma region Close One

			int Sockets::socketClose(SocketInfo * Socket, bool del_socket) {
				std::string msg = "Socket cerrado en " + Socket->IP + ":" + Utils::ltos(Socket->port);
				VServer * VServ = Socket->VServ;
				if (del_socket) socketDelete(Socket);
				if (close(Socket->fd) == -1) return (-1);
				// disconnect and release clients
				Log::log_access(msg, VServ);

				return (0);
			}

			void Sockets::socketClose(VServer * VServ) {
				std::list <SocketInfo>::iterator it = sockets.begin();
				while (it != sockets.end()) {
					if (it->VServ == VServ) {
						socketClose(&(*it), false);
						it = sockets.erase(it);
					} else ++it;
				}
				VServ->status = false;
				Display::Output();
			}

		#pragma endregion

		#pragma region Close Client All

			void Sockets::clientsClose() {
				std::list <Client>::iterator it = clients.begin();
				while (it != clients.end()) {
					if (it->fd != -1) close(it->fd);
					it = clients.erase(it);
				}
			}

		#pragma endregion

	#pragma endregion

	#pragma region Delete

		int Sockets::socketDelete(SocketInfo * Socket) {
			std::list <SocketInfo>::iterator it = sockets.begin();
			while (it != sockets.end()) {
				if (&(*it) == Socket) { sockets.erase(it); return (0); }
				++it;
			}

			return (1);
		}

	#pragma endregion

	#pragma region Accept

		int Sockets::socketAccept(EventInfo * event) {
			sockaddr_in Addr; socklen_t AddrLen = sizeof(Addr);
			int fd = accept(event->fd, (sockaddr *)&Addr, &AddrLen);

			if (fd == -1) { Log::log_error("Error al aceptar la conexión"); return (-1); }

			std::string	IP		= inet_ntoa(Addr.sin_addr);
			int			Port	= ntohs(Addr.sin_port);

			Log::log_access("Conexión aceptada de " + IP);

			//EventInfo client_event(fd, CLIENT, event->Socket, NULL);
			clients.push_back(Client(fd, event->Socket, IP, Port, EventInfo(fd, CLIENT, event->Socket, NULL)));
			
			Client & cli = clients.back();


			event->Socket->clients.push_back(&cli);
			cli.event.client = &cli;

			if (epoll_add(fd, &(cli.event)) == -1) { Log::log_error("Error añadiendo socket al epoll"); close(fd); clients.pop_back(); event->Socket->clients.pop_back(); return (-1); }

			return (fd);
		}

	#pragma endregion

	#pragma region Exists

		bool Sockets::socketExists(const std::string & IP, int port) {
			for (std::list <SocketInfo>::const_iterator it = sockets.begin(); it != sockets.end(); ++it)
				if (it->IP == IP && it->port == port) return (true);
			return (false);
		}

	#pragma endregion

#pragma endregion

#pragma region EPOLL

	#pragma region Start

		int Sockets::epoll_start() {
				if (epoll_fd != -1) epoll_close();

				epoll_fd = epoll_create(1024);
				if (epoll_fd == -1) { Log::log_error("Error creando el epoll file descriptor"); return (1); }

				return (0);
		}

	#pragma endregion

	#pragma region Add

		int Sockets::epoll_add(int fd, EventInfo * event) {
			struct epoll_event epoll_event;

    if (event == NULL) {
        Log::log_error("Error: puntero event es nulo en epoll_add");
        return -1;
    }
			epoll_event.data.ptr = event;
			epoll_event.events = EPOLLIN;																// Monitorizar solo para lecturas (conexiones entrantes)
		
			//return (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event));
			int result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event);
		   	    if (result == -1) {
        Log::log_error("Error en epoll_ctl: " + Utils::ltos(errno));
    } else {
        Log::log_access("epoll_ctl: OK");
    }
			return (result);
		}

#pragma endregion

	#pragma region Close

		void Sockets::epoll_close() {
			if (epoll_fd != -1) close(epoll_fd);
		}

	#pragma endregion

	#pragma region Create Timer FD

		int Sockets::create_timer_fd(int interval_sec) {
			int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
			if (timer_fd == -1) return (-1);

			struct itimerspec new_value;
			memset(&new_value, 0, sizeof(new_value));
			new_value.it_value.tv_sec = interval_sec;		// Tiempo hasta la primera expiración
			new_value.it_interval.tv_sec = interval_sec; 	// Intervalo entre expiraciones

			if (timerfd_settime(timer_fd, 0, &new_value, NULL) == -1) { close(timer_fd); return (-1); }
			return (timer_fd);
		}

	#pragma endregion

#pragma endregion

#pragma region MainLoop

	int Sockets::MainLoop() {
		struct epoll_event events[Settings::MAX_EVENTS];
        int eventCount = epoll_wait(epoll_fd, events, Settings::MAX_EVENTS, 10);
        if (eventCount == -1) { Log::log_error("Error en epoll_wait"); return (-1); }

		for (int i = 0; i < eventCount; ++i) {
            EventInfo * event = static_cast<EventInfo *>(events[i].data.ptr);
			 if (event == NULL) { Log::log_error("Error2 en epoll_wait"); continue; }
			switch (event->type) {
				case SOCKET: 	{ socketAccept(event); break; }
				case CLIENT: 	{ break; }
				case CGI: 		{ break; }
			}
        }
        return (0);
	}

#pragma endregion
