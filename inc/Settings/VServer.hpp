/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:48 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/03 17:46:30 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Location.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <algorithm>																					//	For algorithms like std::sort, std::find, etc.
#include <vector>																						//	For the std::vector container
#include <deque>																						//	For the std::deque container
#include <map>																							//	For the std::map container

class VServer {

	public:

		//	Variables
		std::map<std::string, std::string>	vserver;													//	Values of the current VServer
		std::vector<Location>				location;													//	Locations of the current VServer
		std::deque <std::string>			access;														//	Memory log for Access
		std::deque <std::string>			error;														//	Memory log for Error
		std::deque <std::string>			both;														//	Memory log for Both
		bool								status;														//	Status of the VServer (Started/Stoped)
		
		//	Constructors
		VServer() {}																					//	Default constructor
		VServer(const VServer & src) { *this = src; }													//	Copy constructor
		~VServer() { clear(); }																			//	Destructor (Call clear() before destruction)

		//	Overloads
		VServer &	operator=(const VServer & rhs);														//	Overload for asignation
		bool		operator==(const VServer & rhs) const;												//	Overload for comparison

		//	Methods
		std::string get(const std::string & Key) const;													//	Get a Value from a Key
		void		set(const std::string & Key, const std::string & Value);							//	Add or modify a Key - Value
		void		add(const std::string & Key, const std::string & Value) { set(Key, Value); }		//	Alias for 'set'
		void		del(const std::string & Key);														//	Delete a Key - Value
		void		clear();																			//	Delete all Keys and his Values

		void		set(const Location & Loc);															//	Add or modify a Location
		void		add(const Location & Loc) { set(Loc); }												//	Alias for 'set'
		void		del(const Location & Loc);															//	Delete a Location

};
