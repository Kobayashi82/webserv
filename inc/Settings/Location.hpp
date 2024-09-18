/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:37 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 21:08:26 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Method.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <algorithm>																					//	For std::find()
#include <vector>																						//	For std::vector container
#include <deque>																						//	For std::deque container

#pragma region Location

	class VServer;																						//	Forward declaration of VServer
	class Location {

		public:

			//	Variables
			std::vector <std::pair<std::string, std::string> >	data;									//	Values of the current Location
			std::deque <Method>									method;									//	Method of the current Location
			VServer *											VServ;									//	Pointer to the parent VServer

			//	Constructors
			Location();																					//	Default constructor
			Location(const Location & src);																//	Copy constructor
			~Location();																				//	Destructor

			//	Overloads
			Location &	operator=(const Location & rhs);												//	Overload for assignation
			bool		operator==(const Location & rhs) const;											//	Overload for comparison
			
			//	Methods
			std::string	get(const std::string & Key);													//	Gets a Value from a Key
			void		set(const std::string & Key, const std::string & Value, bool Force = false);	//	Adds or modify a Key - Value pair
			void		add(const std::string & Key, const std::string & Value, bool Force = false);	//	Alias for 'set'
			void		del(const std::string & Key);													//	Deletes a Key - Value pair
			void		clear();																		//	Deletes all Keys and their Values

			void		set(const Method & Met);														//	Adds or modifies a Method
			void		add(const Method & Met);														//	Alias for 'set'
			void		del(const Method & Met);														//	Deletes a Method

	};

#pragma endregion
