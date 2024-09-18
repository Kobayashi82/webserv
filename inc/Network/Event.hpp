/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:20:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 12:59:40 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <vector>																						//	For std::vector container
#include <deque>																						//	For std::deque container
#include <list>																							//	For std::list container
#include <map>																							//	For std::map container

enum e_type { NOTHING, SOCKET, CLIENT, DATA, CGI };	

#pragma region EventInfo

	struct SocketInfo;
	struct EventInfo {

		//	Variables
		int						fd;																		//	File descriptor associated with the event
		int						type;																	//	Type of the event (SOCKET, CLIENT, DATA, CGI)
		SocketInfo *			socket;																	//	Pointer to the associated Client, if applicable
		Client *				client;																	//	Pointer to the associated Client, if applicable

		int						pipe[2];
		size_t					data_size;
		size_t					max_data_size;

		std::vector <char>		read_buffer;															//	Buffer for reading data
   		std::vector <char>		write_buffer;															//	Buffer for writing data
		std::string				request;																//	Request of the client

		std::string 			path;																	//	Path of the file
		bool					no_cache;																//	Do not keep in cache
		bool					close;																	//	Close the conection... please
		std::vector<std::pair<std::string, std::string> > * vserver_data;								//	Data container where the request is server (VServer, Location...)

		//	Constructors
		EventInfo();																					//	Default constructor
		EventInfo(const EventInfo & src);																//	Copy constructor
		EventInfo(int _fd, int _type, SocketInfo * _socket, Client * _client);							//	Parameterized constructor

		//	Overloads
		EventInfo &		operator=(const EventInfo & rhs);												//	Overload for asignation
		bool			operator==(const EventInfo & rhs) const;										//	Overload for comparison

	};

#pragma endregion

class Event {

	public:

		static std::map <int, EventInfo>	events;														//	Map of all events objects
		
		static void			remove_events();
		static void			remove_events(Client * Cli);
		static EventInfo *	get_event(int fd);
		static int			remove_event(int fd);

};