/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:54:41 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 13:18:51 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Settings.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <list>																							//	For std::list container

#include <sys/timerfd.h>																				//	For timerfd to create a FD that triggers events in epoll
#include <sys/epoll.h>																					//	For epoll

class Epoll {

	public:

		static int	create();																			//	Creates epoll, set the FD to 'epoll_fd', create and add 'event_timeout'
		static void	close();																			//	Closes epoll
		static int	add(int fd, bool epollin, bool epollout);											//	Add an event to epoll
		static int	set(int fd, bool epollin, bool epollout);											//	Modify an event in epoll
		static void	remove(int fd);																		//	Removes an event from epoll
		static int	events();																			//	Processes epoll events

	private:

		static int										epoll_fd;										//	File descriptor for epoll
		static int										timeout_fd;										//	EventInfo structure used for generating events in epoll and checking client timeouts

		static const int 								MAX_EVENTS;										//	Maximum number of events that can be handled per iteration by epoll
		static const int 								TIMEOUT_INTERVAL;								//	Interval in seconds between timeout checks for inactive clients

		static int	create_timeout();																	//	Creates a file descriptor for the client timeout checker
		static void check_timeout();																	//	Checks for clients that have timed out

};