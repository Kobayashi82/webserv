/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:54:47 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/29 20:55:03 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

#pragma region Get

	std::string Location::get(const std::string & Key) const {
		std::map<std::string, std::string>::const_iterator it = location.find(Key);
		if (it == location.end()) throw std::out_of_range("Key not found");
		return (it->second);
	}

#pragma endregion

#pragma region Set

	void Location::set(const std::string & Key, const std::string & Value) {
		std::map<std::string, std::string>::iterator it = location.find(Key);
		if (it != location.end()) it->second = Value;
		if (it == location.end()) location[Key] = Value;
	}

#pragma endregion

#pragma region Del

	void Location::del(const std::string & Key) {
		std::map<std::string, std::string>::iterator it = location.find(Key);
		if (it != location.end()) location.erase(it);
	}

#pragma endregion

#pragma region Clear

	void Location::clear() { location.clear(); }

#pragma endregion
