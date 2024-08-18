/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sockets.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:55:43 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/18 15:03:41 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Sockets.hpp"
#include "Client.hpp"

#pragma region Variables

	std::deque <Sockets::SocketInfo>	Sockets::sockets;
	std::deque <Client *>				Sockets::clients;
	int 								Sockets::epoll_fd = -1;

#pragma endregion

#pragma region Constructors

	Sockets::SocketInfo::SocketInfo(int _fd, const std::string & _IP, int _port, VServer * _VServ) : fd(_fd), IP(_IP), port(_port), VServ(_VServ) {}
	Sockets::EventInfo::EventInfo(int _fd, int _type, Sockets::SocketInfo * _Socket, Client * _client) : fd(_fd), type(_type), Socket(_Socket), client(_client) {}

#pragma endregion

#pragma region Sockets

	#pragma region Create

		#pragma region Create All

			bool Sockets::socketCreate() {
				bool success = false;
				
				epoll_fd = epoll_create(1024);
				if (epoll_fd == -1) { Log::log_error("Error creando el epoll file descriptor"); return (false); }

				for (std::deque <VServer>::iterator vserv_it = Settings::vserver.begin(); vserv_it != Settings::vserver.end(); ++vserv_it) {
					for (std::vector <std::pair<std::string, int> >::const_iterator addr_it = vserv_it->addresses.begin(); addr_it != vserv_it->addresses.end(); ++addr_it) {
						if (socketExists(addr_it->first, addr_it->second)) { continue; }

						int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
						if (serverSocket == -1) { Log::log_error("Error creando el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second)); continue; }

						int opt = 1;
						if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) { Log::log_error("Error configurando opciones de socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second)); close(serverSocket); continue; }

						sockaddr_in address;
						std::memset(&address, 0, sizeof(address));
						address.sin_family = AF_INET;
						address.sin_port = htons(addr_it->second);
						inet_pton(AF_INET, addr_it->first.c_str(), &address.sin_addr);

						if (bind(serverSocket, (sockaddr*)&address, sizeof(address)) == -1) {
							//Log::log_error(Utils::ltos(errno) + "Error vinculando el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second));
							close(serverSocket); continue;
						}

						if (listen(serverSocket, SOMAXCONN) == -1) { Log::log_error("Error escuchando en el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second)); close(serverSocket); continue; }

						// Añadir el socket al epoll
						struct epoll_event event;
						event.data.fd = serverSocket;
						event.events = EPOLLIN; // Monitorizar solo para lecturas (conexiones entrantes)
						if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event) == -1) { Log::log_error("Error añadiendo socket al epoll"); close(serverSocket); continue; }

						sockets.push_back(SocketInfo(serverSocket, addr_it->first, addr_it->second, &(*vserv_it)));
						Log::log_access("Socket creado y escuchando en " + addr_it->first + ":" + Utils::ltos(addr_it->second));
						success = true;
					}
				}
				return (success);
			}

		#pragma endregion

		#pragma region Create One

			bool Sockets::socketCreate(VServer & VServ) {
				for (std::vector <std::pair<std::string, int> >::const_iterator addr_it = VServ.addresses.begin(); addr_it != VServ.addresses.end(); ++addr_it) {
					if (socketExists(addr_it->first, addr_it->second)) { continue; }

					int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
					if (serverSocket == -1) { Log::log_error("Error creando el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second)); continue; }

					int opt = 1;
					if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) { Log::log_error("Error configurando opciones de socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second)); close(serverSocket); continue; }

					sockaddr_in address; std::memset(&address, 0, sizeof(address));
					address.sin_family = AF_INET;
					address.sin_port = htons(addr_it->second);
					inet_pton(AF_INET, addr_it->first.c_str(), &address.sin_addr);

					if (bind(serverSocket, (sockaddr *)&address, sizeof(address)) == -1) {
						//Log::log_error(Utils::ltos(errno) + "Error vinculando el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second));
						close(serverSocket); continue;
					}

					if (listen(serverSocket, SOMAXCONN) == -1) { Log::log_error("Error escuchando en el socket para " + addr_it->first + ":" + Utils::ltos(addr_it->second)); close(serverSocket); continue; }


					// Añadir el socket al epoll
					sockets.push_back(SocketInfo(serverSocket, addr_it->first, addr_it->second, &VServ));
					EventInfo event(serverSocket, SOCKET, &(*--sockets.end()), NULL);

					if (epoll_add(serverSocket, event) == -1) { Log::log_error("Error añadiendo socket al epoll"); close(serverSocket); sockets.pop_back(); continue; }

					Log::log_access("Socket creado y escuchando en " + addr_it->first + ":" + Utils::ltos(addr_it->second));
				}
			return (true);
		}

		#pragma endregion

	#pragma endregion

	#pragma region Close

		#pragma region Close All

			void Sockets::socketClose() {
				for (std::deque <SocketInfo>::iterator it = sockets.begin(); it != sockets.end(); ++it) {
					close(it->fd);
					Log::log_access("Socket cerrado en " + it->IP + ":" + Utils::ltos(it->port));
				}
			}

		#pragma endregion

		#pragma region Close One

			void Sockets::socketClose(SocketInfo * Socket) {
				close(Socket->fd);
					Log::log_access("Socket cerrado en " + Socket->IP + ":" + Utils::ltos(Socket->port));
			}

		#pragma endregion

	#pragma endregion

	#pragma region Listen

		int Sockets::socketListen(EventInfo * event) {
			sockaddr_in Addr; socklen_t AddrLen = sizeof(Addr);
			int fd = accept(event->fd, (sockaddr *)&Addr, &AddrLen);

			if (fd == -1) { Log::log_error("Error al aceptar la conexión"); return (-1); }

			std::string	IP		= inet_ntoa(Addr.sin_addr);
			int			Port	= ntohs(Addr.sin_port);

			Log::log_access("Conexión aceptada de " + IP);

			Client client(fd, event->Socket, IP, Port);
			clients.push_back(&client);
			event->Socket->clients.push_back(&client);
			
			EventInfo client_event(fd, CLIENT, event->Socket, *--clients.end());

			if (epoll_add(fd, client_event) == -1) { Log::log_error("Error añadiendo socket al epoll"); close(fd); clients.pop_back(); event->Socket->clients.pop_back(); return (-1); }

			return (fd);
		}

	#pragma endregion

	#pragma region Exists

		bool Sockets::socketExists(const std::string & IP, int port) {
			for (std::deque <SocketInfo>::const_iterator it = sockets.begin(); it != sockets.end(); ++it)
				if (it->IP == IP && it->port == port) return (true);
			return (false);
		}

	#pragma endregion

#pragma endregion

#pragma region EPOLL

	#pragma region Add

		int Sockets::epoll_add(int fd, EventInfo & event) {
			struct epoll_event epoll_event;

			epoll_event.data.ptr = &event;
			epoll_event.data.fd = fd;
			epoll_event.events = EPOLLIN;																// Monitorizar solo para lecturas (conexiones entrantes)
			return (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event));
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
        int eventCount = epoll_wait(epoll_fd, events, Settings::MAX_EVENTS, -1);
        if (eventCount == -1) { Log::log_error("Error en epoll_wait"); return -1; }

		for (int i = 0; i < eventCount; ++i) {
            EventInfo * event = static_cast<EventInfo *>(events[i].data.ptr);

			switch (event->type) {
				case SOCKET: 	{ socketListen(event); break; }
				case CLIENT: 	{ break; }
				case CGI: 		{ break; }
			}
        }
        return (-1);
	}

#pragma endregion
