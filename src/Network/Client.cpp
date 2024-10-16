/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/18 11:28:40 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/01 15:04:43 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Thread.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Client.hpp"
#include "Epoll.hpp"
#include "Communication.hpp"

#pragma region Constructors

    Client::Client(int _fd, SocketInfo * _socket, std::string _ip, int _port) : fd(_fd), socket(_socket), ip(_ip), port(_port) {
		last_activity = std::time(NULL); total_requests = 0;
	}
    Client::Client(const Client & src) : fd(src.fd), socket(src.socket), ip(src.ip), port(src.port), last_activity(src.last_activity), total_requests(src.total_requests) {}


#pragma endregion

#pragma region Overloads

	Client &	Client::operator=(const Client & rhs) {
        if (this != &rhs) { fd = rhs.fd; ip = rhs.ip; port = rhs.port; socket = rhs.socket; last_activity = rhs.last_activity; total_requests = rhs.total_requests; }
		return (*this);
    }

	bool		Client::operator==(const Client & rhs) const {
		return (fd == rhs.fd);
	}

#pragma endregion

#pragma region Time-Out

	void Client::check_timeout(int interval) {
		time_t current_time = std::time(NULL);
		if (difftime(current_time, last_activity) > interval)  remove();
	}

    void Client::update_last_activity() { last_activity = std::time(NULL); }

#pragma endregion

#pragma region Remove

	void	Client::remove(bool from_socket) {
		if (fd != -1) { Event::remove(fd); }

		std::list <Client>::iterator c_it = Communication::clients.begin();
		while (c_it != Communication::clients.end()) {
			if (*this == *c_it) {
				Event::remove(this);

				for (std::list<Client *>::iterator sc_it = socket->clients.begin(); sc_it != socket->clients.end(); ++sc_it) {
					if (*this == **sc_it) { socket->clients.erase(sc_it); break; }
				}

				Communication::clients.erase(c_it);

				if (Thread::get_int(Display::mutex, Communication::total_clients) > 0)
					Thread::dec_int(Display::mutex, Communication::total_clients);

				Display::update(); break;
			} ++c_it;
		}
		if (!from_socket) Socket::do_cleanup = true;
	}

#pragma endregion
