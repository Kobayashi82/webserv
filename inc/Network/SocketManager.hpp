/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:49:50 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/18 00:00:52 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Settings.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <errno.h>

#define MAX_EVENTS 10

class SocketManager {

	public:

		struct SocketInfo {
			int			fd;
			std::string	IP;
			int			port;

			SocketInfo(int s_fd, const std::string & ip, int p);
		};

	    std::vector<std::pair<int, SocketInfo> >	sockets;
	    int											epoll_fd;

		SocketManager();

		bool	socketExists(const std::string & IP, int port) const;
		bool	createSockets();
		bool	createSockets(const VServer & VServ);
		int		listenSockets();
		void	closeSockets();
};