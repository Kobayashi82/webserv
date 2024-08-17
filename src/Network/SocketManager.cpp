/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:55:43 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/18 00:02:45 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketManager.hpp"

    SocketManager::SocketManager() : epoll_fd(-1) {}

	SocketManager::SocketInfo::SocketInfo(int s_fd, const std::string& ip, int p) : fd(s_fd), IP(ip), port(p) {}

	bool SocketManager::socketExists(const std::string & IP, int port) const {
		for (std::vector <std::pair<int, SocketInfo> >::const_iterator it = sockets.begin(); it != sockets.end(); ++it)
			if (it->second.IP == IP && it->second.port == port) return (true);
		return (false);
	}

    bool SocketManager::createSockets() {
    	bool success = false;
		
		epoll_fd = epoll_create(1024);
        if (epoll_fd == -1) { Log::log_error("Error creando el epoll file descriptor"); return (false); }

        for (std::vector <VServer>::const_iterator vserv_it = Settings::vserver.begin(); vserv_it != Settings::vserver.end(); ++vserv_it) {
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

				sockets.push_back(std::make_pair(serverSocket, SocketInfo(serverSocket, addr_it->first, addr_it->second)));
				Log::log_access("Socket creado y escuchando en " + addr_it->first + ":" + Utils::ltos(addr_it->second));
				success = true;
			}
        }
		return (success);
    }

	bool SocketManager::createSockets(const VServer & VServ) {
		for (std::vector <std::pair<std::string, int> >::const_iterator addr_it = VServ.addresses.begin(); addr_it != VServ.addresses.end(); ++addr_it) {
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

			sockets.push_back(std::make_pair(serverSocket, SocketInfo(serverSocket, addr_it->first, addr_it->second)));
			Log::log_access("Socket creado y escuchando en " + addr_it->first + ":" + Utils::ltos(addr_it->second));
		}
	return (true);
}

    int SocketManager::listenSockets() {
        struct epoll_event events[MAX_EVENTS];
        int eventCount = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); // Esperar indefinidamente por eventos

        if (eventCount == -1) { Log::log_error("Error en epoll_wait"); return -1; }

        for (int i = 0; i < eventCount; ++i) {
            int eventSocket = events[i].data.fd;

            // Aceptar la conexión entrante
            sockaddr_in clientAddress;
            socklen_t clientAddrLen = sizeof(clientAddress);
            int clientSocket = accept(eventSocket, (sockaddr *)&clientAddress, &clientAddrLen);

            if (clientSocket == -1) { Log::log_error("Error al aceptar la conexión"); continue; }

           Log::log_access("Conexión aceptada de " + std::string(inet_ntoa(clientAddress.sin_addr)));
            return clientSocket; // Devolver el socket del cliente aceptado
        }

        return (-1);
    }

    void SocketManager::closeSockets() {
        for (std::vector<std::pair<int, SocketInfo> >::iterator it = sockets.begin(); it != sockets.end(); ++it) {
            close(it->first);
			Log::log_access("Socket cerrado en " + it->second.IP + ":" + Utils::ltos(it->second.port));
		}
        if (epoll_fd != -1) close(epoll_fd);
    }
