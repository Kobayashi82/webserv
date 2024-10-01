/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 10:59:39 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/01 15:04:36 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"
#include "Display.hpp"
#include "Thread.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Communication.hpp"

#pragma region Variables

	std::list<SocketInfo>					Socket::sockets;											//	List of all SocketInfo objects
	int										Socket::ask_socket = 0;										//	Flag indicating the request to create or close all sockets		(Used when Key_W is pressed)
	std::list<std::pair <VServer *, int> >	Socket::socket_action_list;									//	List of VServers to enable or disable							(Used when Key_V is pressed)
	bool									Socket::do_cleanup = false;									//	Flag indicating that a cleanup of sockets is necessary

	std::vector<std::string>				Socket::network_interfaces;									//	List of all network interfaces

#pragma endregion

#pragma region SocketInfo

	#pragma region Constructors

		SocketInfo::SocketInfo(int _fd, const std::string & _ip, int _port, VServer * _VServ) : fd(_fd), ip(_ip), port(_port), VServ(_VServ) {}
		SocketInfo::SocketInfo(const SocketInfo & src) { *this = src; }

	#pragma endregion

	#pragma region Overloads

		SocketInfo & SocketInfo::operator=(const SocketInfo & rhs) {
			if (this != &rhs) { fd = rhs.fd; ip = rhs.ip; port = rhs.port; VServ = rhs.VServ; clients = rhs.clients; }
			return (*this);
		}

		bool SocketInfo::operator==(const SocketInfo & rhs) const {
			return (fd == rhs.fd && ip == rhs.ip && port == rhs.port && VServ == rhs.VServ && clients == rhs.clients);
		}

	#pragma endregion

#pragma endregion

#pragma region Sockets

	#pragma region Create

		#pragma region Create All

			int Socket::create(bool create_network) {
				if (create_network) NetworkInterfaces();
				bool nothing_created = true;

				if (Thread::get_bool(Display::mutex, Settings::global.status) == false) return (1);

				for (std::deque <VServer>::iterator vserv_it = Settings::vserver.begin(); vserv_it != Settings::vserver.end(); ++vserv_it)
					if (!create(&(*vserv_it))) nothing_created = false;

				return (nothing_created);
			}

		#pragma endregion

		#pragma region Create VServer

			int Socket::create(VServer * VServ) {
				//	If the server or the virtual server is disabled do nothing
				bool nothing_created = true;
				if (Thread::get_bool(Display::mutex, Settings::global.status) == false  || Settings::global.bad_config || VServ->bad_config || Thread::get_bool(Display::mutex, VServ->force_off)) return (1);

				//	For every IP address in the virtual server create a socket
				for (std::vector <std::pair<std::string, int> >::const_iterator addr_it = VServ->addresses.begin(); addr_it != VServ->addresses.end(); ++addr_it) {
					if (!isNetworkInterface(addr_it->first)) continue;

					//	Create socket
					int fd = socket(AF_INET, SOCK_STREAM, 0);
					if (fd == -1) { error_msg(addr_it->first, addr_it->second, VServ, SK_CREATE); continue; }
					//Utils::NonBlocking_FD(fd);

					//	Configure socket
					int options = 1;
					if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options)) == -1) { error_msg(addr_it->first, addr_it->second, VServ, SK_CONFIGURE); ::close(fd); continue; }

					//	Initialize the socket address structure with the IP address and port
					sockaddr_in address; std::memset(&address, 0, sizeof(address));
					address.sin_family = AF_INET;
					address.sin_port = htons(addr_it->second);
					if (addr_it->first == "0.0.0.0")	address.sin_addr.s_addr = INADDR_ANY;
					else 								inet_pton(AF_INET, addr_it->first.c_str(), &address.sin_addr);

					//	Link the address to the socket (0.0.0.0 links to all available network interfaces)
					if (bind(fd, (sockaddr *)&address, sizeof(address)) == -1) { error_msg(addr_it->first, addr_it->second, VServ, SK_BIND); ::close(fd); continue; }

					//	Listen on the address for incoming connections
					if (listen(fd, SOMAXCONN) == -1) { error_msg(addr_it->first, addr_it->second, VServ, SK_LISTEN); ::close(fd); continue; }

					//	Add the socket FD to EPOLL
					if (Epoll::add(fd, true, false) == -1) { error_msg(addr_it->first, addr_it->second, VServ,SK_EPOLL); ::close(fd); continue; }

					// Create a SocketInfo, an EventInfo and add them to Socket::sockets and Socket::events
					sockets.push_back(SocketInfo(fd, addr_it->first, addr_it->second, VServ));
					Event::events[fd] = EventInfo(fd, SOCKET, &sockets.back(), NULL);

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

	#pragma endregion

	#pragma region Close

		#pragma region Close All

			void Socket::close() {
				std::list<SocketInfo>::iterator s_it = sockets.begin();
				while (s_it != sockets.end()) {
					if (Thread::get_bool(Display::mutex, s_it->VServ->status)) Thread::set_bool(Display::mutex, s_it->VServ->status, false);
					SocketInfo current = *s_it; ++s_it;
					remove(current);
				}

				Thread::set_int(Display::mutex, Communication::total_clients, 0);
			}

		#pragma endregion

		#pragma region Close VServer

			void Socket::close(VServer * VServ) {
				std::list<SocketInfo>::iterator s_it = sockets.begin();
				while (s_it != sockets.end()) {
					if (s_it->VServ == VServ) {
						SocketInfo current = *s_it; ++s_it;
						remove(current);
					} else ++s_it;
				}
				Thread::set_bool(Display::mutex, VServ->status, false);
			}

		#pragma endregion

	#pragma endregion

	#pragma region Remove

		void Socket::remove() {
		    while (!sockets.empty()) remove(sockets.front());
		}

		void Socket::remove(SocketInfo & socket) {
			if (socket.fd != -1) { Epoll::remove(socket.fd); ::close(socket.fd); } Event::remove(socket.fd);
			std::list <SocketInfo>::iterator s_it = sockets.begin();
			while (s_it != sockets.end()) {
				if (*s_it == socket) {
					std::list <Client *>::iterator c_it = socket.clients.begin();
					while (c_it != socket.clients.end()) {
						std::list <Client *>::iterator curr_c_it = c_it;
						Client * current = *c_it; ++c_it;
						current->remove(true);
						socket.clients.erase(curr_c_it);
					}
					sockets.erase(s_it);
					break;
				}
				++s_it;
			}
		}

	#pragma endregion

	#pragma region Accept

		void Socket::accept(EventInfo * event) {
			sockaddr_in Addr; socklen_t AddrLen = sizeof(Addr);
			int fd = ::accept(event->fd, (sockaddr *)&Addr, &AddrLen);
			if (fd == -1) { error_msg(event->socket->ip, event->socket->port, event->socket->VServ, SK_ACCEPT); return; }
			//Utils::NonBlocking_FD(fd);

			std::string	ip		= inet_ntoa(Addr.sin_addr);
			int			port	= ntohs(Addr.sin_port);

			Communication::clients.push_back(Client(fd, event->socket, ip, port));
			Event::events[fd] = EventInfo(fd, CLIENT, event->socket, NULL);
			Event::events[fd].client = &Communication::clients.back();
			event->socket->clients.push_back(&Communication::clients.back());

			if (Epoll::add(fd, true, false) == -1) { Event::events[fd].client->remove(); return; }

			Thread::inc_int(Display::mutex, Communication::total_clients);
		}

	#pragma endregion

	#pragma region Status

		int Socket::server_status() {
			bool update_display = false;
			bool recreate_sockets = false;

			Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

			if (socket_action_list.size() > 0) {
				std::list <std::pair<VServer *, int> >::iterator it = socket_action_list.begin();
				while (it != socket_action_list.end()) {

					Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

						if (it->second == CREATE) create(it->first);
						if (it->second == CLOSE) { close(it->first); recreate_sockets = true; }
						
					Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

					it = socket_action_list.erase(it);
				}
				update_display = true;
			}

			Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);
	
			if (recreate_sockets) create();
			if (update_display) Display::update();

			Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

				if (ask_socket == 1) { ask_socket = 0; Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK); create(); Display::update(); return (1); }
				if (ask_socket == 2) { ask_socket = 0; Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK); close();  Display::update(); return (2); }

			Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

			return (0);
		}

	#pragma endregion
		
	#pragma region Clean Up

		void Socket::cleanup_socket() {
			if (!do_cleanup) return;																	//	Only clean up if necessary

			std::list<SocketInfo>::iterator s_it = sockets.begin();
			while (s_it != sockets.end()) {																//	Iterate over sockets
				std::list<Client *>::iterator sc_it = s_it->clients.begin();
				while (sc_it != s_it->clients.end()) { bool found = false;								//	Iterate over socket clients				
					std::list<Client>::iterator c_it = Communication::clients.begin();
					while (c_it != Communication::clients.end()) {										//	Iterate over clients
						if (*c_it == **sc_it) { found = true; break; }									//	client = socket client
						++c_it;
					}
					if (!found)				sc_it = s_it->clients.erase(sc_it);							// If socket client is not in clients, remove it
					else					++sc_it;
				}
				++s_it;
			}
		}

	#pragma endregion

	#pragma region Network Interfaces

		void Socket::NetworkInterfaces() {
			struct ifaddrs *ifaddr, *ifa;
			char addr[INET_ADDRSTRLEN];

			if (getifaddrs(&ifaddr) == -1) return;

			for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
				if (ifa->ifa_addr == NULL) continue;

				if (ifa->ifa_addr->sa_family == AF_INET) {
					struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
					inet_ntop(AF_INET, &(sa->sin_addr), addr, INET_ADDRSTRLEN);
					network_interfaces.push_back(std::string(addr));
				}
			}

			freeifaddrs(ifaddr);
		}

		bool Socket::isNetworkInterface(const std::string & ip) {
			if (ip == "0.0.0.0") return (true);

			for (std::vector<std::string>::iterator it = network_interfaces.begin(); it != network_interfaces.end(); ++it)
				if (*it == ip) return (true);

			return (false);
		}

	#pragma endregion

	//	DISABLED FOR NOW

	#pragma region Error Messages

		void Socket::error_msg(const std::string & ip, const int port, VServer * VServ, int type) {
			return;																						//	Disabled for now
			switch (type) {
				case SK_CREATE:
					Log::log("Socket " + ip + ":" + Utils::ltos(port) + " cannot be created", Log::MEM_ERROR, VServ); break;
				case SK_CONFIGURE:
					Log::log("Socket " + ip + ":" + Utils::ltos(port) + " cannot be configured", Log::MEM_ERROR, VServ); break;
				case SK_BIND:
					Log::log("Socket " + ip + ":" + Utils::ltos(port) + " cannot be binded", Log::MEM_ERROR, VServ); break;
				case SK_LISTEN:
					Log::log("Socket " + ip + ":" + Utils::ltos(port) + " cannot listen", Log::MEM_ERROR, VServ); break;
				case SK_EPOLL:
					Log::log("Socket " + ip + ":" + Utils::ltos(port) + " cannot be added to EPOLL", Log::MEM_ERROR, VServ); break;
				case SK_ACCEPT:
					Log::log("Socket " + ip + ":" + Utils::ltos(port) + " cannot accept a client", Log::MEM_ERROR, VServ); break;
			}
		}

	#pragma endregion

	//	DISABLED FOR NOW
	
#pragma endregion
