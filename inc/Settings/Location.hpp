/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:53:37 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/27 19:01:26 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <map>

class Location {

	public:

		//	Variables
		std::map<std::string, std::string> location;

		//	Constructors
		Location(const Location & src) { *this = src; }
		~Location() { clear(); }

		//	Overloads
		Location & operator=(const Location & rhs) { if (this != &rhs) { location = rhs.location; } return (*this); }
		bool operator==(const Location & rhs) const { return (location == rhs.location); }
		
		//	Location
		std::string get(const std::string & Key) const;
		void set(const std::string & Key, const std::string & Value);
		void add(const std::string & Key, const std::string & Value) { set(Key, Value); }
		void del(const std::string & Key);

		void clear();

};
