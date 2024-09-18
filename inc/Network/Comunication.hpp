/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Comunication.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 12:44:04 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 12:50:13 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"
#include "Security.hpp"
#include "Cache.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <list>																							//	For std::list container
#include <map>																							//	For std::map container

#include <sys/socket.h>

#pragma once

struct EventInfo;
class Client;
class Comunication {

	public:

		//	Variables
		enum e_socket_action { CREATE, CLOSE };															//	Enumeration for socket actions									(Used when Key_W and Key_V are pressed)

		static std::list <Client>						clients;										//	List of all Client objects
		static Cache									cache;											//	Used to store cached data, such as files or HTML responses

		static int										total_clients;									//	Total number of clients conected
		static long										read_bytes;										//	Total number of bytes downloaded by the server
		static long										write_bytes;									//	Total number of bytes uploaded by the server

		static const size_t								CHUNK_SIZE;										//	Size of the buffer for read and write operations

		//	Comunications (Work in progress)

		static int	read_data(EventInfo * event);
		static int	read_client(EventInfo * event);

		static void	write_client(EventInfo * event);

		static void	process_data(EventInfo * event, std::string data);
		static void	process_data(EventInfo * event, size_t data_size);
		static void	process_request(EventInfo * event);
		static void	process_response(EventInfo * event);

		//	Resolve request
		static void	resolve_request(const std::string host, const std::string method, std::string path, EventInfo * event);

};