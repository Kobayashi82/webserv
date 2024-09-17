/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Method.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:37 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/17 13:38:40 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <vector>																						//	For std::vector container

class VServer;
class Location;
class Method {

	public:

		//	Variables
		std::vector <std::pair<std::string, std::string> >	data;										//	Values of the current Method
		VServer *											VServ;										//	Pointer to the parent VServer
		Location *											Loc;										//	Pointer to the parent Location

		//	Constructors
		Method();																						//	Default constructor
		Method(const Method & src);																		//	Copy constructor
		~Method();																						//	Destructor

		//	Overloads
		Method &	operator=(const Method & rhs);														//	Overload for asignation
		bool		operator==(const Method & rhs) const;												//	Overload for comparison
		
		//	Methods
		std::string	get(const std::string & Key);														//	Get a Value from a Key
		void		set(const std::string & Key, const std::string & Value, bool Force = false);		//	Add or modify a Key - Value
		void		add(const std::string & Key, const std::string & Value, bool Force = false);		//	Alias for 'set'
		void		del(const std::string & Key);														//	Delete a Key - Value
		void		clear();																			//	Delete all Keys and his Values

};
