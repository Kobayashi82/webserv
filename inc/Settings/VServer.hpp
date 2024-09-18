/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:48 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 21:09:35 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Log.hpp"
#include "Location.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <algorithm>																					//	For std::find()
#include <vector>																						//	For std::vector container
#include <deque>																						//	For std::deque container

#pragma region VServer

	class VServer {

		public:

			//	Variables
			std::vector <std::pair<std::string, std::string> >	data;									//	Values of the current VServer
			std::vector <std::pair<std::string, int> >			addresses;								//	Values of the current VServer
			std::deque <Location>								location;								//	Locations of the current VServer
			std::deque <Method>									method;									//	Method of the current VServer
			std::vector <std::string>							config;									//	Settings in a vector of the current VServer
			bool												config_displayed;						//	Indicates whether the log or settings are displayed
			size_t												config_index;							//	Current index of the settings
			size_t												log_index;								//	Current index of the log
			bool												autolog;								//	Auto-scroll logs
			Log													log;									//	Logs (access, error and both)
			bool												status;									//	Status of the VServer (Started/Stopped)
			bool												force_off;								//	The VServer is disabled by the user;
			bool												bad_config;								//	The configuration is bad (disable the VServer)
			
			//	Constructors
			VServer();																					//	Default constructor
			VServer(const VServer & src);																//	Copy constructor
			~VServer();																					//	Destructor

			//	Overloads
			VServer &	operator=(const VServer & rhs);													//	Overload for assignation
			bool		operator==(const VServer & rhs) const;											//	Overload for comparison

			//	Methods
			std::string get(const std::string & Key);													//	Gets a Value from a Key
			void		set(const std::string & Key, const std::string & Value, bool Force = false);	//	Adds or modifies a Key - Value pair
			void		add(const std::string & Key, const std::string & Value, bool Force = false);	//	Alias for 'set'
			void		del(const std::string & Key);													//	Deletes a Key - Value pair
			void		clear();																		//	Deletes all Keys and their Values
			void		clear_logs();																	//	Deletes all logs entries

			void		set_address(const std::string & IP, const int & Port);							//	Adds an address (IP - Port)
			void		add_address(const std::string & IP, const int & Port);							//	Alias for 'set_address'
			void		del_address(const std::string & IP, const int & Port);							//	Deletes an address (IP - Port)

			void		set(const Location & Loc);														//	Adds or modifies a Location
			void		add(const Location & Loc);														//	Alias for 'set'
			void		del(const Location & Loc);														//	Deletes a Location

			void		set(const Method & Met);														//	Adds or modifies a Method
			void		add(const Method & Met);														//	Alias for 'set'
			void		del(const Method & Met);														//	Deletes a Method

	};

#pragma endregion
