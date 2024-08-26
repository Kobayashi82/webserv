/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:49:48 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/26 22:06:40 by vzurera-         ###   ########.fr       */
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
		Net::EventInfo		event;																		//	EventInfo associated with this client
    	time_t				last_activity;																//	Last activity timespan  (for keep-alive)
		long				total_requests;																//	Maximum request allowed (for keep-alive)

		std::vector <char> 	read_buffer;																//	Buffer for reading data
    	std::vector <char> 	write_buffer;																//	Buffer for writing data
		size_t				read_pos;																	//	Current position in the read buffer
		size_t				write_pos;																	//	Current position in the write buffer

		//	Constructors
		Client(int _fd, Net::SocketInfo * _socket, std::string _IP, int _port, Net::EventInfo _event);	//	Parameterized constructor
		Client(const Client & Cli);																		//	Copy constructor

		//	Overloads
		Client &	operator=(const Client & rhs);														//	Overload for asignation
		bool		operator==(const Client & rhs) const;												//	Overload for comparison

		//	Methods
		void	check_timeout(int interval);															//	Checks if the client has timed out
		void	update_last_activity();																	//	Updates the client last activity timestamp

		void	remove(bool no_msg = false);															//	Removes the client by closing the connection and cleaning up associated resources

};
