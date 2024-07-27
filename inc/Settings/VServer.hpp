/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:48 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/27 19:01:19 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Location.hpp"
#include <algorithm> 
#include <vector>

class VServer {

	public:

		//	Variables
		std::map<std::string, std::string> vserver;
		std::vector<Location> location;
		
		//	Constructors
		VServer(const VServer & src) { *this = src; }
		~VServer() { clear(); }

		//	Overloads
		VServer & operator=(const VServer & rhs) { if (this != &rhs) { vserver = rhs.vserver; location = rhs.location; } return (*this); }
		bool operator==(const VServer & rhs) const { return (vserver == rhs.vserver && location == rhs.location); }

		//	VServer
		std::string get(const std::string & Key) const;
		void set(const std::string & Key, const std::string & Value);
		void add(const std::string & Key, const std::string & Value) { set(Key, Value); }
		void del(const std::string & Key);

		//	Location
		void set(const Location & Loc);
		void add(const Location & Loc) { set(Loc); }
		void del(const Location & Loc);

		void clear();

};
