/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 10:58:42 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/23 20:15:33 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <vector>																						//	For std::vector container
#include <list>																							//	For std::list container

#include <arpa/inet.h>																					//	For sockets and address conversion
#include <ifaddrs.h>																					//	For network interfaces

#pragma region SocketInfo

	class VServer;																						//	Forward declaration of VServer
	struct SocketInfo {

		//	Variables
		int						fd;																		//	File descriptor for the socket
		std::string				ip;																		//	IP address of the socket
		int						port;																	//	Port number of the socket
		VServer *				VServ;																	//	Pointer to the associated VServer
		std::list <Client *>	clients;																//	List of clients connected to this socket

		//	Constructors
		SocketInfo(const SocketInfo & src);																//	Copy constructor
		SocketInfo(int _fd, const std::string & _ip, int _port, VServer * _VServ);						//	Parameterized constructor

		//	Overloads
		SocketInfo & 	operator=(const SocketInfo & rhs);												//	Overload for assignation
		bool 			operator==(const SocketInfo & rhs) const;										//	Overload for comparison

	};

#pragma endregion

#pragma region Socket

	struct EventInfo;																					//	Forward declaration of EventInfo
	class Socket {

		public:

			//	Enumerators
			enum e_action { CREATE = 101, CLOSE = 102 };
			enum e_error { SK_CREATE = 140, SK_CONFIGURE = 141, SK_BIND = 142, SK_LISTEN = 143, SK_EPOLL = 144, SK_ACCEPT = 145 };

			//	Variables
			static std::list <SocketInfo>					sockets;									//	List of all SocketInfo objects
			static int										ask_socket;									//	Flag indicating the request to create or close all sockets		(Used when Key_W is pressed)
			static std::list <std::pair <VServer *, int> >	socket_action_list;							//	List of VServers to enable or disable							(Used when Key_V is pressed)
			static bool										do_cleanup;									//	Flag indicating that a cleanup of sockets is necessary

			//	Methods
			static int	create(bool create_network = false);											//	Creates all sockets from all VServers
			static int	create(VServer * VServ);														//	Creates all sockets from a VServer

			static void	close();																		//	Closes all sockets
			static void	close(VServer * VServ);															//	Closes all sockets associated with a VServer

			static void remove();																		//	Removes all sockets
			static void remove(SocketInfo & socket);													//	Removes all sockets associated with a VServer

			static void accept(EventInfo * event);														//	Accept a new connection (client)

			static int	server_status();																//	Check if there are any pending changes to the server or virtual servers
		
			static void	NetworkInterfaces();

		private:

			//	Variables
			static std::vector<std::string>					network_interfaces;							//	List of all network interfaces

			//	Methods
			static bool	isNetworkInterface(const std::string & ip);										//	Checks if an IP address is a valid network interface
			static bool	exists(const std::string & ip, int port);										//	Checks if an IP address and port already exist in a socket
			static void	error_msg(const std::string & ip, const int port, VServer * VServ, int type);	//	Logs socket error messages

	};

#pragma endregion
