/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sockets.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:49:50 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/18 15:00:43 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Settings.hpp"

#include <iostream>
#include <deque>																						//	For std::deque container
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
class Sockets {

	public:

		enum e_type { SOCKET, CLIENT, CGI };

		struct SocketInfo {
			int						fd;
			std::string				IP;
			int						port;
			VServer *				VServ;
			std::deque <Client *>	clients;

			SocketInfo(int _fd, const std::string & _IP, int _port, VServer * _VServ);
		};

		struct EventInfo {
			int				fd;
			int				type;
			SocketInfo *	Socket;
			Client *		client;

			EventInfo(int _fd, int _type, Sockets::SocketInfo * _Socket, Client * _client);
		};

	    static std::deque <SocketInfo>	sockets;
		static std::deque <Client *>	clients;
	    static int						epoll_fd;

		static int	MainLoop();

		static int	epoll_add(int fd, EventInfo & event);
		static void	epoll_close();

		static bool	socketExists(const std::string & IP, int port);
		static bool	socketCreate();
		static bool	socketCreate(VServer & VServ);
		static int	socketListen(EventInfo * event);
		static void	socketClose();
		static void	socketClose(SocketInfo * Socket);

		static int	create_timer_fd(int interval = Settings::TIMEOUT_INTERVAL);

};