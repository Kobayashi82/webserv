/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Net.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:49:50 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/21 20:15:30 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Settings.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <deque>																						//	For std::deque container
#include <list>																							//	For std::list container

#include "unistd.h"
#include <arpa/inet.h>																					//	For sockets and address conversion
#include <sys/timerfd.h>																				//	For timerfd to create a FD that triggers events in epoll
#include <sys/epoll.h>																					//	For epoll

class Client;
class Net {

	public:

		#pragma region EventInfo

		struct SocketInfo;
		struct EventInfo {

			//	Variables
			int						fd;																	//	File descriptor associated with the event
			int						type;																//	Type of the event (SOCKET, CLIENT, DATA, CGI, TIMEOUT)
			SocketInfo *			socket;																//	Pointer to the associated SocketInfo, if applicable
			Client *				client;																//	Pointer to the associated Client, if applicable

			std::vector <char>		read_buffer;														//	Buffer for reading data
    		std::vector <char>		write_buffer;														//	Buffer for writing data
		    size_t					read_pos;															//	Current position in the read buffer
		    size_t					write_pos;															//	Current position in the write buffer

			std::list <EventInfo>	event_data;															//	List of related EventInfo objects. Used for files and CGIs

			//	Constructors
			EventInfo();																				//	Default constructor
			EventInfo(int _fd, int _type, Net::SocketInfo * _socket, Client * _client);					//	Parameterized constructor

			//	Overloads
			EventInfo &		operator=(const EventInfo & rhs);											//	Overload for asignation
			bool			operator==(const EventInfo & rhs);											//	Overload for comparison

		};

		#pragma endregion

		#pragma region SocketInfo

		struct SocketInfo {

			//	Variables
			int							fd;																//	File descriptor for the socket
			std::string					IP;																//	IP address of the socket
			int							port;															//	Port number of the socket
			EventInfo					event;															//	EventInfo associated with this socket
			VServer *					VServ;															//	Pointer to the associated virtual server
			std::list <Client *>		clients;														//	List of clients associated with this socket

			//	Constructors
			SocketInfo(int _fd, const std::string & _IP, int _port, EventInfo _event, VServer * _VServ);//	Parameterized constructor

			//	Overloads
			SocketInfo & 	operator=(const SocketInfo & rhs);											//	Overload for asignation
			bool 			operator==(const SocketInfo & rhs);											//	Overload for comparison

			//	Methods
			void 			remove();																	//	Remove a SocketInfo object based on a reference

		};

		#pragma endregion

		//	Variables
	    static std::list <SocketInfo>	sockets;														//	List of all SocketInfo objects
		static std::list <Client>		clients;														//	List of all Client objects

		static long						read_bytes;														//	Total number of bytes downloaded by the server
		static long						write_bytes;													//	Total number of bytes uploaded by the server

		//	Socket
		static int	socket_create_all();																//	Creates all sockets from all VServers
		static int	socket_create(VServer * VServ);														//	Creates all sockets from a VServer
		static void	socket_close_all();																	//	Closes all sockets
		static void	socket_close(VServer * VServ);														//	Closes all sockets associated with a VServer

		//	EPOLL
		static int	epoll__create();																	//	Creates epoll, set the FD to 'epoll_fd', create and add 'event_timeout'
		static void	epoll_close();																		//	Closes epoll
		static int	epoll_add(EventInfo * event);														//	Adds an event to epoll
		static int	epoll_edit(EventInfo * event, bool epollin, bool epollout);
		static void	epoll_del(EventInfo * event);														//	Removes an event from epoll
		static int	epoll_events();																		//	Processes epoll events

	private:

		//	Variables
		enum e_type { SOCKET, CLIENT, DATA, CGI, TIMEOUT };												//	Enumeration for event types used in EventInfo
		
		static int									epoll_fd;											//	File descriptor for epoll
		static EventInfo							event_timeout;										//	EventInfo structure used for generating events in epoll and checking client timeouts

		static const int 							MAX_EVENTS;											//	Maximum number of events that can be handled per iteration by epoll
		static const int							EPOLL_BUFFER_SIZE;									//	Size of the buffer for read and write operations
		static const int 							TIMEOUT_INTERVAL;									//	Interval in seconds between timeout checks for inactive clients

		static const int							KEEP_ALIVE_TIMEOUT;									//	Timeout in seconds for keep-alive (if a client is inactive for this amount of time, the connection will be closed)
		static const int							KEEP_ALIVE_REQUEST;									//	Maximum request for keep-alive (if a client exceeds this number of requests, the connection will be closed)

		//	Socket
		static void	socket_accept(EventInfo * event);													//	Accepts a new connection
		static bool	socket_exists(const std::string & IP, int port);									//	Checks if a socket with the given IP address and port exists
		
		//	EPOLL
		static int	create_timeout();																	//	Creates a file descriptor for the client timeout checker
		static void check_timeout();																	//	Checks for clients that have timed out

		static int read_request(EventInfo * event);
		static void write_response(EventInfo * event);
		static void	process_request(EventInfo * event);
		static void	process_response(EventInfo * event);

};
