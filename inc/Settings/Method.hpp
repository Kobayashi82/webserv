/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Method.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:37 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 21:08:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <vector>																						//	For std::vector container

#pragma region Method

	class VServer;																						//	Forward declaration of VServer
	class Location;																						//	Forward declaration of Location
	class Method {

		public:

			//	Variables
			std::vector <std::pair<std::string, std::string> >	data;									//	Values of the current Method
			VServer *											VServ;									//	Pointer to the parent VServer
			Location *											Loc;									//	Pointer to the parent Location

			//	Constructors
			Method();																					//	Default constructor
			Method(const Method & src);																	//	Copy constructor
			~Method();																					//	Destructor

			//	Overloads
			Method &	operator=(const Method & rhs);													//	Overload for assignation
			bool		operator==(const Method & rhs) const;											//	Overload for comparison
			
			//	Methods
			std::string	get(const std::string & Key);													//	Gets a Value from a Key
			void		set(const std::string & Key, const std::string & Value, bool Force = false);	//	Adds or modifies a Key - Value pair
			void		add(const std::string & Key, const std::string & Value, bool Force = false);	//	Alias for 'set'
			void		del(const std::string & Key);													//	Deletes a Key - Value pair
			void		clear();																		//	Deletes all Keys and their Values

	};

#pragma endregion
