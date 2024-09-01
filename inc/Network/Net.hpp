/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Net.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/17 21:49:50 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/01 10:34:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Settings.hpp"
#include "Security.hpp"
#include "Cache.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <deque>																						//	For std::deque container
#include <list>																							//	For std::list container

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
			int						type;																//	Type of the event (SOCKET, CLIENT, DATA, CGI)
			SocketInfo *			socket;																//	Pointer to the associated Client, if applicable
			Client *				client;																//	Pointer to the associated Client, if applicable

			int						data_fd;
			size_t					data_size;

			int						pipe[2];
			//size_t					data_size;
			size_t					max_data_size;

			std::vector <char>		read_buffer;														//	Buffer for reading data
    		std::vector <char>		write_buffer;														//	Buffer for writing data

			std::string 			path;																//	Path of the file
			bool					no_cache;															//	Do not keep in cache

			//	Constructors
			EventInfo();																				//	Default constructor
			EventInfo(const EventInfo & src);															//	Copy constructor
			EventInfo(int _fd, int _type, Net::SocketInfo * _socket, Client * _client);					//	Parameterized constructor

			//	Overloads
			EventInfo &		operator=(const EventInfo & rhs);											//	Overload for asignation
			bool			operator==(const EventInfo & rhs) const;									//	Overload for comparison

			//int 			remove();																	//	Remove the corresponding owner of the EventInfo (client, socket or event_data)

		};

		//static EventInfo * create_event_data(int _fd, int _type, EventInfo * parent);

		#pragma endregion

		#pragma region SocketInfo

		struct SocketInfo {

			//	Variables
			int						fd;																	//	File descriptor for the socket
			std::string				IP;																	//	IP address of the socket
			int						port;																//	Port number of the socket
			VServer *				VServ;																//	Pointer to the associated VServer
			std::list <Client *>	clients;															//	List of clients associated with this socket

			//	Constructors
			SocketInfo(const SocketInfo & src);															//	Copy constructor
			SocketInfo(int _fd, const std::string & _IP, int _port, VServer * _VServ);					//	Parameterized constructor

			//	Overloads
			SocketInfo & 	operator=(const SocketInfo & rhs);											//	Overload for asignation
			bool 			operator==(const SocketInfo & rhs) const;									//	Overload for comparison

			//	Methods
			void 			remove();																	//	Remove a SocketInfo object

		};

		#pragma endregion

		//	Variables
		enum e_socket_action { CREATE, CLOSE };															//	Enumeration for socket actions									(Used when Key_W and Key_V are pressed)

	    static std::list <SocketInfo>					sockets;										//	List of all SocketInfo objects
		static std::list <Client>						clients;										//	List of all Client objects
		static std::map <int, EventInfo>				events;											//	Map of all events objects
		static std::map <int, int>						mierdas;										//	Map of all mierdas
		static Cache									cache;											//	Used to store cached data, such as files or HTML responses


		static int										total_clients;									//	Total number of clients conected
		static long										read_bytes;										//	Total number of bytes downloaded by the server
		static long										write_bytes;									//	Total number of bytes uploaded by the server

		static bool										do_cleanup;										//
		static int										ask_socket;										//	Flag indicating the request to create or close all sockets		(Used when Key_W is pressed)

		static std::list <std::pair <VServer *, int> >	socket_action_list;								//	List of VServers to enable or disable							(Used when Key_V is pressed)

		//	Socket
		static int	socket_create_all();																//	Creates all sockets from all VServers
		static int	socket_create(VServer * VServ);														//	Creates all sockets from a VServer
		static void	socket_close_all();																	//	Closes all sockets
		static void	socket_close(VServer * VServ);														//	Closes all sockets associated with a VServer

		//	EPOLL
		static int	epoll__create();																	//	Creates epoll, set the FD to 'epoll_fd', create and add 'event_timeout'
		static void	epoll_close();																		//	Closes epoll
		static int	epoll_add(int fd, bool epollin, bool epollout);										//	Add an event to epoll
		static int	epoll_set(int fd, bool epollin, bool epollout);										//	Modify an event in epoll
		static void	epoll_del(int fd);																	//	Removes an event from epoll
		static int	epoll_events();																		//	Processes epoll events

		static void			remove_events();
		static void			remove_events(Client * Cli);
		static EventInfo *	get_event(int fd);
		static int			remove_event(int fd);
		static void			cleanup_socket();

	private:


		#pragma region ResolveInfo

		struct ResolveInfo {

			//	Variables
			EventInfo *				event;																//	Pointer to the associated EventInfo
			VServer *				VServ;																//	Pointer to the associated VServer
			Location *				Loc;																//	Pointer to the associated Location
			Method *				Met;																//	Pointer to the associated Method

			//	Constructors
			//ResolveInfo(const ResolveInfo & src);														//	Copy constructor
			//ResolveInfo();																			//	Parameterized constructor

			//	Overloads
			//ResolveInfo & 	operator=(const ResolveInfo & rhs);										//	Overload for asignation
			//bool 			operator==(const ResolveInfo & rhs) const;									//	Overload for comparison

		};

		#pragma endregion

		//	Variables
		enum e_type { NOTHING, SOCKET, CLIENT, DATA, CGI };												//	Enumeration for event types used in EventInfo
		
		static int										epoll_fd;										//	File descriptor for epoll
		static int										timeout_fd;										//	EventInfo structure used for generating events in epoll and checking client timeouts

		static const int 								MAX_EVENTS;										//	Maximum number of events that can be handled per iteration by epoll
		static const size_t								CHUNK_SIZE;										//	Size of the buffer for read and write operations
		static const int 								TIMEOUT_INTERVAL;								//	Interval in seconds between timeout checks for inactive clients

		static const int								KEEP_ALIVE_TIMEOUT;								//	Timeout in seconds for keep-alive (if a client is inactive for this amount of time, the connection will be closed)
		static const int								KEEP_ALIVE_REQUEST;								//	Maximum request for keep-alive (if a client exceeds this number of requests, the connection will be closed)

		//	Socket
		static void	socket_accept(EventInfo * event);													//	Accepts a new connection
		static bool	socket_exists(const std::string & IP, int port);									//	Checks if a socket with the given IP address and port exists
		static int	server_status();																	//

		//	EPOLL
		static int	create_timeout();																	//	Creates a file descriptor for the client timeout checker
		static void check_timeout();																	//	Checks for clients that have timed out


		//	Comunications (Work in progress)
		static int	read_data(EventInfo * event);
		static int	read_client(EventInfo * event);

		static void	write_client(EventInfo * event);

		static void	process_data(EventInfo * event, std::string data);
		static void	process_data(EventInfo * event, size_t data_size);
		static void	process_request(EventInfo * event, std::string request);
		static void	process_response(EventInfo * event);

		//	Resolve request
		static void	resolve_request(const std::string host, const std::string method, std::string path, EventInfo * event);

};
