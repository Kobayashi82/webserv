/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Communication.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 12:44:04 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/21 12:26:31 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Settings.hpp"
#include "Security.hpp"
#include "Cache.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <list>																							//	For std::list container

#pragma region Communication

	struct EventInfo;																					//	Forward declaration of EventInfo
	class Client;																						//	Forward declaration of Client
	class Communication {

		public:

			//	Variables
			static std::list <Client>						clients;									//	List of all Client objects
			static Cache									cache;										//	Used to store cached data, such as files or HTML responses

			static int										total_clients;								//	Total number of clients conected
			static size_t									read_bytes;									//	Total number of bytes downloaded by the server
			static size_t									write_bytes;								//	Total number of bytes uploaded by the server

			static const size_t								CHUNK_SIZE;									//	Size of the buffer for read and write operations
			static const size_t								HEADER_MAXSIZE;								//	Maximum size allowed for the header (8 KB by default)

			//	Methods
			static int	read_client(EventInfo * event);													//	Read from data the client's socket
			static void	write_client(EventInfo * event);												//	Write data to the client's socket

			static int	read_data(EventInfo * event);													//	Read data from a file
			
			static int	read_cgi(EventInfo * event);													//	Read data from a CGI
			static void	write_cgi(EventInfo * event);													//	Write data to a CGI

	};

#pragma endregion
