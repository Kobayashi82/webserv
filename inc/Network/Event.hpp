/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:20:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/01 13:11:28 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <vector>																						//	For std::vector container
#include <map>																							//	For std::map container
#include <ctime>
#include <csignal>

#pragma region EventInfo

	//	Enumerators
	enum e_type { NOTHING, SOCKET, CLIENT, DATA, CGI };

	struct SocketInfo;																					//	Forward declaration of SocketInfo
	class VServer;																						//	Forward declaration of VServer
	class Location;																						//	Forward declaration of Location
	struct EventInfo {

		//	Variables
		int													fd;											//	File descriptor associated with the event
		int													type;										//	Type of the event (SOCKET, CLIENT, DATA, CGI)
		SocketInfo *										socket;										//	Pointer to the associated Socket (if applicable)
		Client *											client;										//	Pointer to the associated Client (if applicable)

		std::vector <char>									read_buffer;								//	Buffer for reading data
   		std::vector <char>									write_buffer;								//	Buffer for writing data

		int													pipe[2];									//	Pipe used to redirect the file's FD using splice(), so it can be added to EPOLL

		std::string 										read_path;									//	Path of a file
		size_t												read_size;									//	Total bytes read from a file
		size_t												read_maxsize;								//	The file size
		int													read_info;									//	Info on the file size (0 = known, 1 = unknow, 2 = no more data)

		std::string 										write_path;									//	Path of a file
		size_t												write_size;									//	Total bytes read from a file
		size_t												write_maxsize;								//	The file size
		int													write_info;									//	Info on the file size (0 = known, 1 = unknow, 2 = no more data)
		int													cgi_fd;

		std::string											header;
		std::map<std::string, std::string>					header_map;
		std::map<std::string, std::string>					response_map;
		size_t												response_size;
		std::string											response_method;
		time_t												mod_time;
		size_t												filesize;
		
		size_t												body_size;
		size_t												body_maxsize;
		bool												no_cache;									//	Do not keep in cache
		bool												close;										//	Close the conection... please

		int													redirect_status;
		struct timeval										response_time;
		time_t												last_activity;
		int													pid;

		VServer *											VServ;
		Location *											Loc;
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
