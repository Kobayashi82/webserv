/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:37 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 15:05:13 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <map>																							//	For the std::map container

class Location {

	public:

		//	Variables
		std::map<std::string, std::string>	location;													//	Values of the current Location

		//	Constructors
		Location();																						//	Default constructor
		Location(const Location & src);																	//	Copy constructor
		~Location();																					//	Destructor (Call clear() before destruction)

		//	Overloads
		Location &	operator=(const Location & rhs);													//	Overload for asignation
		bool		operator==(const Location & rhs) const;												//	Overload for comparison
		
		//	Methods
		std::string	get(const std::string & Key) const;													//	Get a Value from a Key
		void		set(const std::string & Key, const std::string & Value);							//	Add or modify a Key - Value
		void		add(const std::string & Key, const std::string & Value) { set(Key, Value); }		//	Alias for 'set'
		void		del(const std::string & Key);														//	Delete a Key - Value
		void		clear();																			//	Delete all Keys and his Values

};
