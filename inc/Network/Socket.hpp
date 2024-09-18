/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 10:58:42 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 13:28:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <vector>																						//	For std::vector container
#include <deque>																						//	For std::deque container
#include <list>																							//	For std::list container

#include <arpa/inet.h>																					//	For sockets and address conversion

class VServer;

#pragma region SocketInfo

	struct SocketInfo {

		//	Variables
		int						fd;																		//	File descriptor for the socket
		std::string				ip;																		//	IP address of the socket
		int						port;																	//	Port number of the socket
		VServer *				VServ;																	//	Pointer to the associated VServer
		std::list <Client *>	clients;																//	List of clients conected to this socket

		//	Constructors
		SocketInfo(const SocketInfo & src);																//	Copy constructor
		SocketInfo(int _fd, const std::string & _ip, int _port, VServer * _VServ);						//	Parameterized constructor

		//	Overloads
		SocketInfo & 	operator=(const SocketInfo & rhs);												//	Overload for asignation
		bool 			operator==(const SocketInfo & rhs) const;										//	Overload for comparison

	};

#pragma endregion

struct EventInfo;
class Socket {

	public:

		enum e_socket_action { CREATE = 101, CLOSE = 102 };	
		enum e_type { NOTHING, SOCKET, CLIENT, DATA, CGI };	
		enum e_socket_error { SK_CREATE, SK_CONFIGURE, SK_BIND, SK_LISTEN, SK_EPOLL };						//	Enumaration for socket errors
		

		static std::list <SocketInfo>					sockets;										//	List of all SocketInfo objects
		static int										ask_socket;										//	Flag indicating the request to create or close all sockets		(Used when Key_W is pressed)
		static std::list <std::pair <VServer *, int> >	socket_action_list;								//	List of VServers to enable or disable							(Used when Key_V is pressed)
		static bool										do_cleanup;										//

		static int	create();																			//	Creates all sockets from all VServers
		static int	create(VServer * VServ);															//	Creates all sockets from a VServer

		static void	close();																			//	Closes all sockets
		static void	close(VServer * VServ);																//	Closes all sockets associated with a VServer

		static void remove();
		static void remove(SocketInfo & socket);

		static void accept(EventInfo * event);

		static int	server_status();																	//
		static void	cleanup_socket();
		
	private:

		static bool	socket_exists(const std::string & ip, int port);
		static void	error_msg(std::vector <std::pair<std::string, int> >::const_iterator addr_it, VServer * VServ, int type);

};