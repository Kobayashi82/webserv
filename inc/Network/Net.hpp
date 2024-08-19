/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Net.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:49:50 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/19 15:35:25 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Settings.hpp"

#include <iostream>
#include <deque>																						//	For std::deque container
#include <list>																							//	For std::list container
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <errno.h>

class Client;
class Net {

	public:

		enum e_type { SOCKET, CLIENT, CGI };

		struct SocketInfo;
		struct EventInfo {
			int						fd;
			int						type;
			SocketInfo *			Socket;
			Client *				client;

			EventInfo(int _fd, int _type, Net::SocketInfo * _Socket, Client * _client);
			EventInfo & operator=(const EventInfo & rhs);
		};

		struct SocketInfo {
			int							fd;
			std::string					IP;
			int							port;
			EventInfo					event;
			VServer *					VServ;
			std::deque <Client *>		clients;

			SocketInfo(int _fd, const std::string & _IP, int _port, EventInfo _event, VServer * _VServ);
			SocketInfo & operator=(const SocketInfo & rhs);
		};


	    static std::list <SocketInfo>	sockets;
		static std::list <Client>		clients;
	    static int						epoll_fd;

		static int	MainLoop();

		static int	epoll_start();
		static int	epoll_add(int fd, EventInfo * event);
		static void	epoll_close();

		static bool	socketExists(const std::string & IP, int port);
		static int	socketCreate();
		static int	socketCreate(VServer * VServ);
		static int	socketAccept(EventInfo * event);
		static void	socketClose();
		static int	socketClose(SocketInfo * Socket, bool del_socket = true);
		static void	socketClose(VServer * VServ);
		static void	clientsClose();
		static int	socketDelete(SocketInfo * Socket);

		static int	create_timer_fd(int interval = Settings::TIMEOUT_INTERVAL);

};