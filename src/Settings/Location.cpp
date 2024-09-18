/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:54:47 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 19:57:39 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

#pragma region Constructors

    Location::Location() { VServ = NULL; }
    Location::Location(const Location & src) { *this = src; }
    Location::~Location() { clear(); VServ = NULL; }

#pragma endregion

#pragma region Overloads

	Location &	Location::operator=(const Location & rhs) { if (this != &rhs) { data = rhs.data; method = rhs.method; VServ = rhs.VServ; } return (*this); }
	bool		Location::operator==(const Location & rhs) const { return (data == rhs.data && method == rhs.method) && VServ == rhs.VServ; }

#pragma endregion

#pragma region Location

    #pragma region Get

        std::string Location::get(const std::string & Key) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
				if (it->first == Key) return (it->second);
			return ("");
        }

    #pragma endregion

    #pragma region Set/Add

        void Location::set(const std::string & Key, const std::string & Value, bool Force) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
				if (!Force && it->first == Key) { it->second = Value; return; }

			data.push_back(std::make_pair(Key, Value));
        }

		void Location::add(const std::string & Key, const std::string & Value, bool Force) { set(Key, Value, Force); }

    #pragma endregion

    #pragma region Del

        void Location::del(const std::string & Key) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
				if (it->first == Key) { data.erase(it); }
        }

    #pragma endregion

	#pragma region Clear

		void Location::clear() {
			for (std::deque <Method>::iterator it = method.begin(); it != method.end(); ++it) it->clear();
			data.clear(); method.clear();
		}

	#pragma endregion

#pragma endregion

#pragma region Method

    #pragma region Set/Add

        void Location::set(const Method & Met) {
            std::deque <Method>::iterator it = std::find(method.begin(), method.end(), Met);
            if (it == method.end()) method.push_back(Met);
        }

		void Location::add(const Method & Met) { set(Met); }

    #pragma endregion

    #pragma region Del

        void Location::del(const Method & Met) {
            std::deque <Method>::iterator it = std::find(method.begin(), method.end(), Met);
            if (it != method.end()) { it->clear(); method.erase(it); }
        }

    #pragma endregion

#pragma endregion
