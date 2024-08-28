/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:49:48 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/29 00:07:52 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Net.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <vector>																						//	For std::vector container

class Client {

	public:

		//	Variables
		int					fd;																			//	File descriptor associated with the client
		Net::SocketInfo *	socket;																		//	Pointer to the associated SocketInfo
		std::string			IP;																			//	IP address of the client
		int					port;																		//	Port number of the client
    	time_t				last_activity;																//	Last activity time point	(for keep-alive)
		long				total_requests;																//	Maximum request allowed		(for keep-alive)

		//	Constructors
		Client(int _fd, Net::SocketInfo * _socket, std::string _IP, int _port);							//	Parameterized constructor
		Client(const Client & Cli);																		//	Copy constructor

		//	Overloads
		Client &	operator=(const Client & rhs);														//	Overload for asignation
		bool		operator==(const Client & rhs) const;												//	Overload for comparison

		//	Methods
		void	check_timeout(int interval);															//	Checks if the client has timed out
		void	update_last_activity();																	//	Updates the client last activity timestamp
		void	remove(bool from_socket = false);														//	Close the connection, removes the client and cleaning up resources

};
