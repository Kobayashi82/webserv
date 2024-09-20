/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:20:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/20 14:22:23 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <vector>																						//	For std::vector container
#include <map>																							//	For std::map container

#pragma region EventInfo

	//	Enumerators
	enum e_type { NOTHING, SOCKET, CLIENT, DATA, CGI };

	struct SocketInfo;																					//	Forward declaration of SocketInfo
	struct EventInfo {

		//	Variables
		int													fd;											//	File descriptor associated with the event
		int													type;										//	Type of the event (SOCKET, CLIENT, DATA, CGI)
		SocketInfo *										socket;										//	Pointer to the associated Socket (if applicable)
		Client *											client;										//	Pointer to the associated Client (if applicable)

		std::vector <char>									read_buffer;								//	Buffer for reading data
   		std::vector <char>									write_buffer;								//	Buffer for writing data
		std::string											request;									//	Request from the client

		int													pipe[2];									//	Pipe used to redirect the file's FD using splice(), so it can be added to EPOLL
		std::string 										file_path;									//	Path of a file
		size_t												file_read;									//	Total bytes read from a file
		size_t												file_size;									//	The file size
		int													file_info;									//	Info on the file size (0 = known, 1 = unknow, 2 = no more data)
		int													cgi_fd;

		std::string											header;
		std::map<std::string, std::string>					header_map;
		bool												no_cache;									//	Do not keep in cache
		bool												close;										//	Close the conection... please

		time_t												last_activity;

		std::vector<std::pair<std::string, std::string> > * vserver_data;								//	Data container from which the request is served (Global, VServer, Location)

		//	Constructors
		EventInfo();																					//	Default constructor
		EventInfo(const EventInfo & src);																//	Copy constructor
		EventInfo(int _fd, int _type, SocketInfo * _socket, Client * _client);							//	Parameterized constructor

		//	Overloads
		EventInfo &		operator=(const EventInfo & rhs);												//	Overload for assignation
		bool			operator==(const EventInfo & rhs) const;										//	Overload for comparison

	};

#pragma endregion

#pragma region Event

	class Event {

		public:

			//	Variables
			static std::map <int, EventInfo>	events;													//	Map of all events objects
			
			//	Methods
			static EventInfo *	get(int fd);															//	Gets the event associated with an FD
			static void			remove();																//	Removes all events
			static void			remove(Client * Cli);													//	Removes all events associated with a client
			static int			remove(int fd);															//	Removes an event associated with an FD

			static void			check_timeout(int interval = 5);
			static void			update_last_activity(int fd);


	};

#pragma endregion
