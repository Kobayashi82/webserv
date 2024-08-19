/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:49:48 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/19 15:39:39 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Settings.hpp"
#include "Net.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <vector>																						//	For std::vector container

#include <sys/timerfd.h>
#include <ctime>

class Client {

	public:

		//	Variables
		int						fd;
		Net::SocketInfo *	Socket;
		std::string				IP;
		int						port;
		Net::EventInfo		event;
    	time_t					last_activity;
		std::vector <char> 		read_buffer;
    	std::vector <char> 		write_buffer;

		//	Constructors
		Client(int _fd, Net::SocketInfo * _Socket, std::string _IP, int _port, Net::EventInfo _event);
		Client(const Client & Cli);

		//	Overloads
		Client &	operator=(const Client & rhs);														//	Overload for asignation
		bool		operator==(const Client & rhs) const;												//	Overload for comparison

		//	Methods
		void	check_timeout(int interval_sec = Settings::TIMEOUT_INTERVAL);

};