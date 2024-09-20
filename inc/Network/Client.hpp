/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:49:48 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/21 00:03:36 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout

#pragma region Client

	struct SocketInfo;																					//	Forward declaration of SocketInfo
	class Client {

		public:

			//	Variables
			int				fd;																			//	File descriptor associated with the client
			SocketInfo *	socket;																		//	Pointer to the associated SocketInfo
			std::string		ip;																			//	IP address of the client
			int				port;																		//	Port number of the client
			time_t			last_activity;																//	Last activity time point	(for keep-alive)
			long			total_requests;																//	Number of requests 			(for keep-alive)

			//	Constructors
			Client(int _fd, SocketInfo * _socket, std::string _ip, int _port);							//	Parameterized constructor
			Client(const Client & Cli);																	//	Copy constructor

			//	Overloads
			Client &	operator=(const Client & rhs);													//	Overload for assignation
			bool		operator==(const Client & rhs) const;											//	Overload for comparison

			//	Methods
			void	check_timeout(int interval);														//	Checks if the client has timed out
			void	update_last_activity();																//	Updates the client last activity timestamp
			void	remove(bool from_socket = false);													//	Closes the connection, removes the client and cleans up resources

	};

#pragma endregion
