/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Method.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:54:47 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/15 19:08:15 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Method.hpp"

#pragma region Constructors

    Method::Method() {}
    Method::Method(const Method & src) { *this = src; }
    Method::~Method() { clear(); }

#pragma endregion

#pragma region Overloads

	Method &	Method::operator=(const Method & rhs) { if (this != &rhs) { method = rhs.method; } return (*this); }
	bool		Method::operator==(const Method & rhs) const { return (method == rhs.method); }

#pragma endregion

#pragma region Method

    #pragma region Get

        std::string Method::get(const std::string & Key) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = method.begin(); it != method.end(); ++it)
			if (it->first == Key) return (it->second);
			return ("");
        }

    #pragma endregion

    #pragma region Set/Add

        void Method::set(const std::string & Key, const std::string & Value, bool Force) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = method.begin(); it != method.end(); ++it)
			if (!Force && it->first == Key) { it->second = Value; return; }
			method.push_back(std::make_pair(Key, Value));
        }

		void Method::add(const std::string & Key, const std::string & Value, bool Force) { set(Key, Value, Force); }

    #pragma endregion

    #pragma region Del

        void Method::del(const std::string & Key) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = method.begin(); it != method.end(); ++it)
			if (it->first == Key) { method.erase(it); }
        }

    #pragma endregion

	#pragma region Clear

		void Method::clear() { method.clear(); }

	#pragma endregion

#pragma endregion
