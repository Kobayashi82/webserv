/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/18 11:28:40 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/19 15:40:19 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Net.hpp"

#pragma region Constructors

    Client::Client(int _fd, Net::SocketInfo * _Socket, std::string _IP, int _port, Net::EventInfo _event) : fd(_fd), Socket(_Socket), IP(_IP), port(_port), event(_event) {}
    Client::Client(const Client & src) : fd(src.fd), Socket(src.Socket), IP(src.IP), port(src.port), event(src.event), last_activity(src.last_activity), read_buffer(src.read_buffer), write_buffer(src.write_buffer) {}


#pragma endregion

#pragma region Overloads

	Client &	Client::operator=(const Client & rhs) {
        if (this != &rhs) {
            fd = rhs.fd; IP = rhs.IP; port = rhs.port; Socket = rhs.Socket; last_activity = rhs.last_activity;
			read_buffer = rhs.read_buffer; write_buffer = rhs.write_buffer; event = rhs.event;
        } return (*this);
    }

	bool		Client::operator==(const Client & rhs) const {
		return (fd == rhs.fd && IP == rhs.IP && port == rhs.port && Socket == rhs.Socket && last_activity == rhs.last_activity
				&& read_buffer == rhs.read_buffer && write_buffer == rhs.write_buffer);
	}

#pragma endregion

#pragma region Check Time-Out

	void Client::check_timeout(int interval) {
		time_t current_time = std::time(NULL);
		if (difftime(current_time, last_activity) > interval) {
			close(fd);																						//	La conexión ha superado el timeout, ciérrala
			Log::log_access("The client in " + IP + ":" + Utils::ltos(port) + " closed because time-out");
			// Net::removeClient(this);
		}
	}

#pragma endregion
