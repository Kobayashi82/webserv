/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Communication.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 12:44:04 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/21 00:13:39 by vzurera-         ###   ########.fr       */
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

			//	Methods
			static int	read_client(EventInfo * event);
			static void	write_client(EventInfo * event);

			static int	read_data(EventInfo * event);
			
			static int	read_cgi(EventInfo * event);
			static void	write_cgi(EventInfo * event);


			static void	process_data(EventInfo * event, std::string data);
			static void	process_data(EventInfo * event, size_t data_size);
			static void	process_request(EventInfo * event);
			static void	process_response(EventInfo * event);

			static void	resolve_request(const std::string host, const std::string method, std::string path, EventInfo * event);

	};

#pragma endregion
