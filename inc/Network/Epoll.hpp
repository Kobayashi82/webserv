/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:54:41 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/21 00:03:47 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Settings.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout

#include <sys/timerfd.h>																				//	For timerfd to create a FD that triggers events in EPOLL
#include <sys/epoll.h>																					//	For EPOLL

#pragma region Epoll

	class Epoll {

		public:

			//	Methods
			static int	create();																		//	Creates and initializes EPOLL and timeout
			static void	close();																		//	Closes EPOLL
			static int	add(int fd, bool epollin, bool epollout);										//	Adds an event to EPOLL
			static int	set(int fd, bool epollin, bool epollout);										//	Modifies an event in EPOLL
			static void	remove(int fd);																	//	Removes an event from EPOLL
			static int	events();																		//	Processes EPOLL events

		private:

			//	Variables
			static int										epoll_fd;									//	File descriptor for EPOLL
			static int										timeout_fd;									//	File descriptor used to generating events in EPOLL and checking client timeouts

			static const int 								MAX_EVENTS;									//	Maximum number of events that can be handled per iteration by EPOLL
			static const int 								TIMEOUT_INTERVAL;							//	Interval in seconds between timeout checks for inactive clients

			//	Methods
			static int	create_timeout();																//	Creates the file descriptor for timeout
			static void	clients_timeout();																//	Checks for clients that have timed out
			static void check_timeout();																//	Checks for time outs (Clients, Events and Cache)

	};

#pragma endregion
