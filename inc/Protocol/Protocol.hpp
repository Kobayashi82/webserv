/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Protocol.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 12:49:42 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/28 13:13:06 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Security.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <vector>																						//	For std::vector container
#include <map>																							//	For std::map container

#include <fcntl.h>																						//	For open and splice
#include <dirent.h>																						//	For opendir and closedir. Used to generate a list of the content of a directory

#pragma region Protocol

	struct EventInfo;																					//	Forward declaration of EventInfo
	class Protocol {

		public:

			//	Request
			static int	parse_code(EventInfo * event, int code);										//	Check if the server must manage the response code
			static void	parse_request(EventInfo * event);												//	Create a map container with the values necessary to generate a response
			static void	parse_variables(EventInfo * event);												//	Adds header variables to 'header_map'
			static int	parse_header(EventInfo * event);												//	Parse a header and create a map container with its Key-Value pairs

			static void	process_request(EventInfo * event);												//	Process a request to determine the type of response to return

			//	Response
			static void response_error(EventInfo * event);												//	Return an error message in HTML format
			static void response_redirect(EventInfo * event);											//	Return a redirect message in HTML format
			static void response_directory(EventInfo * event);											//	Return the contents of a directory in HTML format
			static void response_file(EventInfo * event);												//	Return the contents of a file
			static void	response_cgi(EventInfo * event);												//	Return the result of a CGI

			//	Methods
			static void process_response(EventInfo * event);											//	Process the response type and return the appropriate response

			static int	file_cache(EventInfo * event, std::string & path);								//	Gets a file from cache (if exists)
			static void variables_cgi(EventInfo * event, std::vector<std::string> & cgi_vars);			//	Creates variables for a CGI

	};

#pragma endregion
