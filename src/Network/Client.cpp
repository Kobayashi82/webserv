/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/18 11:28:40 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/29 00:08:34 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Client.hpp"
#include "Net.hpp"
#include "Thread.hpp"
#include "Log.hpp"

#pragma region Constructors

    Client::Client(int _fd, Net::SocketInfo * _socket, std::string _IP, int _port) : fd(_fd), socket(_socket), IP(_IP), port(_port) {
		last_activity = std::time(NULL); total_requests = 0;
	}
    Client::Client(const Client & src) : fd(src.fd), socket(src.socket), IP(src.IP), port(src.port), last_activity(src.last_activity), total_requests(src.total_requests) {}


#pragma endregion

#pragma region Overloads

	Client &	Client::operator=(const Client & rhs) {
        if (this != &rhs) {
            fd = rhs.fd; IP = rhs.IP; port = rhs.port; socket = rhs.socket; last_activity = rhs.last_activity; total_requests = rhs.total_requests;
        } return (*this);
    }

	bool		Client::operator==(const Client & rhs) const {
		return (fd == rhs.fd && IP == rhs.IP && port == rhs.port && socket == rhs.socket && last_activity == rhs.last_activity && total_requests == rhs.total_requests);
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
		Net::epoll_del(fd); if (fd != -1) close(fd);
		Net::events.erase(fd);

		// if (socket && socket->clients.size() > 0) {
		// 	std::list <Client *>::iterator s_it = socket->clients.begin();
		// 	while (s_it != socket->clients.end()) {
		// 		if (*s_it == this) { s_it = socket->clients.erase(s_it); break; }
		// 		++s_it;
		// 	}
		// }

		// socket = NULL;

		if (Net::clients.size() == 0) return;
		std::list <Client>::iterator c_it = Net::clients.begin();
		while (c_it != Net::clients.end()) {
			if (*this == *c_it) {
				Net::remove_events(this);
				Net::clients.erase(c_it);

				Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

					Net::total_clients--;

				Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

				Display::update(); break;
			} ++c_it;
		}
		if (!from_socket) Net::do_cleanup = true;
	}

#pragma endregion
