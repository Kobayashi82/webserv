/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Method.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:54:47 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 19:57:36 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Method.hpp"

#pragma region Constructors

    Method::Method() { VServ = NULL; Loc = NULL; }
    Method::Method(const Method & src) { *this = src; }
    Method::~Method() { clear(); Loc = NULL; }

#pragma endregion

#pragma region Overloads

	Method &	Method::operator=(const Method & rhs) { if (this != &rhs) { data = rhs.data; Loc = rhs.Loc; VServ = rhs.VServ; } return (*this); }
	bool		Method::operator==(const Method & rhs) const { return (data == rhs.data && Loc == rhs.Loc && VServ == rhs.VServ); }

#pragma endregion

#pragma region Method

    #pragma region Get

        std::string Method::get(const std::string & Key) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
				if (it->first == Key) return (it->second);
			return ("");
        }

    #pragma endregion

    #pragma region Set/Add

        void Method::set(const std::string & Key, const std::string & Value, bool Force) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
				if (!Force && it->first == Key) { it->second = Value; return; }
			data.push_back(std::make_pair(Key, Value));
        }

		void Method::add(const std::string & Key, const std::string & Value, bool Force) { set(Key, Value, Force); }

    #pragma endregion

    #pragma region Del

        void Method::del(const std::string & Key) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
				if (it->first == Key) { data.erase(it); }
        }

    #pragma endregion

	#pragma region Clear

		void Method::clear() { data.clear(); }

	#pragma endregion

#pragma endregion
