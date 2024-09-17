/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:48 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/17 13:09:28 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Log.hpp"
#include "Location.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <algorithm>																					//	For std::find()
#include <vector>																						//	For std::vector container
#include <deque>																						//	For std::deque container

class VServer {

	public:

		//	Variables
		std::vector <std::pair<std::string, std::string> >	data;										//	Values of the current VServer
		std::vector <std::pair<std::string, int> >			addresses;									//	Values of the current VServer
		std::deque <Location>								location;									//	Locations of the current VServer
		std::deque <Method>									method;										//	Method of the current VServer
		std::vector <std::string>							config;										//	Settings in a vector of the current VServer
		bool												config_displayed;							//	Is the log or the settings displayed
		size_t												config_index;								//	Current index of the settings
		size_t												log_index;									//	Current index of the log
		bool												autolog;									//	Auto scroll logs
		Log													log;										//	Logs (access, error and both)
		bool												status;										//	Status of the VServer (Started/Stoped)
		bool												force_off;									//	The VServer is disabled by user;
		bool												bad_config;									//	The configuration is bad (disable the VServer)
		
		//	Constructors
		VServer();																						//	Default constructor
		VServer(const VServer & src);																	//	Copy constructor
		~VServer();																						//	Destructor

		//	Overloads
		VServer &	operator=(const VServer & rhs);														//	Overload for asignation
		bool		operator==(const VServer & rhs) const;												//	Overload for comparison

		//	Methods
		std::string get(const std::string & Key);														//	Get a Value from a Key
		void		set(const std::string & Key, const std::string & Value, bool Force = false);		//	Add or modify a Key - Value
		void		add(const std::string & Key, const std::string & Value, bool Force = false);		//	Alias for 'set'
		void		del(const std::string & Key);														//	Delete a Key - Value
		void		clear();																			//	Delete all Keys and his Values
		void		clear_logs();																		//	Delete all logs entries

		void		set_address(const std::string & IP, const int & Port);								//	Add an address (IP - Port)
		void		add_address(const std::string & IP, const int & Port);								//	Alias for 'set_address'
		void		del_address(const std::string & IP, const int & Port);								//	Delete an address (IP - Port)

		void		set(const Location & Loc);															//	Add or modify a Location
		void		add(const Location & Loc);															//	Alias for 'set'
		void		del(const Location & Loc);															//	Delete a Location

		void		set(const Method & Met);															//	Add or modify a Method
		void		add(const Method & Met);															//	Alias for 'set'
		void		del(const Method & Met);															//	Delete a Method

};
