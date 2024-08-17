/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:37 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/17 16:40:19 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Method.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <algorithm>																					//	For std::find()
#include <vector>																						//	For std::vector container

class Location {

	public:

		//	Variables
		std::vector <std::pair<std::string, std::string> >	data;										//	Values of the current Location
		std::vector <Method>								method;										//	Locations of the current VServer

		//	Constructors
		Location();																						//	Default constructor
		Location(const Location & src);																	//	Copy constructor
		~Location();																					//	Destructor

		//	Overloads
		Location &	operator=(const Location & rhs);													//	Overload for asignation
		bool		operator==(const Location & rhs) const;												//	Overload for comparison
		
		//	Methods
		std::string	get(const std::string & Key);														//	Get a Value from a Key
		void		set(const std::string & Key, const std::string & Value, bool Force = false);		//	Add or modify a Key - Value
		void		add(const std::string & Key, const std::string & Value, bool Force = false);		//	Alias for 'set'
		void		del(const std::string & Key);														//	Delete a Key - Value
		void		clear();																			//	Delete all Keys and his Values

		void		set(const Method & Met);															//	Add or modify a Method
		void		add(const Method & Met);															//	Alias for 'set'
		void		del(const Method & Met);															//	Delete a Method

};
