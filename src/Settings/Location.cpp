/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:54:47 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/15 19:08:02 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

#pragma region Constructors

    Location::Location() {}
    Location::Location(const Location & src) { *this = src; }
    Location::~Location() { clear(); }

#pragma endregion

#pragma region Overloads

	Location &	Location::operator=(const Location & rhs) { if (this != &rhs) { location = rhs.location; method = rhs.method; } return (*this); }
	bool		Location::operator==(const Location & rhs) const { return (location == rhs.location && method == rhs.method); }

#pragma endregion

#pragma region Location

    #pragma region Get

        std::string Location::get(const std::string & Key) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = location.begin(); it != location.end(); ++it)
			if (it->first == Key) return (it->second);
			return ("");
        }

    #pragma endregion

    #pragma region Set/Add

        void Location::set(const std::string & Key, const std::string & Value, bool Force) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = location.begin(); it != location.end(); ++it)
			if (!Force && it->first == Key) { it->second = Value; return; }
			location.push_back(std::make_pair(Key, Value));
        }

		void Location::add(const std::string & Key, const std::string & Value, bool Force) { set(Key, Value, Force); }

    #pragma endregion

    #pragma region Del

        void Location::del(const std::string & Key) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = location.begin(); it != location.end(); ++it)
			if (it->first == Key) { location.erase(it); }
        }

    #pragma endregion

	#pragma region Clear

		void Location::clear() {
			for (std::vector <Method>::iterator it = method.begin(); it != method.end(); ++it) it->clear();
			location.clear(); method.clear();
		}

	#pragma endregion

#pragma endregion

#pragma region Method

    #pragma region Set/Add

        void Location::set(const Method & Met) {
            std::vector <Method>::iterator it = std::find(method.begin(), method.end(), Met);
            if (it == method.end()) method.push_back(Met);
        }

		void Location::add(const Method & Met) { set(Met); }

    #pragma endregion

    #pragma region Del

        void Location::del(const Method & Met) {
            std::vector <Method>::iterator it = std::find(method.begin(), method.end(), Met);
            if (it != method.end()) { it->clear(); method.erase(it); }
        }

    #pragma endregion

#pragma endregion
