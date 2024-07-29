/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:48 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/29 21:11:26 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Location.hpp"
#include <iostream>
#include <algorithm> 
#include <vector>

class VServer {

	public:

		//	Variables
		std::map<std::string, std::string>	vserver;
		std::vector<Location>				location;
		
		//	Constructors
		VServer() {}
		VServer(const VServer & src) { *this = src; }
		~VServer() { clear(); }

		//	Overloads
		VServer &	operator=(const VServer & rhs) { if (this != &rhs) { vserver = rhs.vserver; location = rhs.location; } return (*this); }
		bool		operator==(const VServer & rhs) const { return (vserver == rhs.vserver && location == rhs.location); }

		//	Methods
		std::string get(const std::string & Key) const;
		void		set(const std::string & Key, const std::string & Value);
		void		add(const std::string & Key, const std::string & Value) { set(Key, Value); }
		void		del(const std::string & Key);
		void		clear();

		void		set(const Location & Loc);
		void		add(const Location & Loc) { set(Loc); }
		void		del(const Location & Loc);


};
