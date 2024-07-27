/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:54:47 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/27 15:11:16 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

//	Location

std::string Location::get(const std::string & Key) const {
    std::map<std::string, std::string>::const_iterator it = location.find(Key);
    if (it == location.end()) throw std::out_of_range("Key not found");
	return (it->second);
}

void Location::set(const std::string & Key, const std::string & Value) {
    std::map<std::string, std::string>::iterator it = location.find(Key);
	if (it != location.end()) it->second = Value;
	if (it == location.end()) location[Key] = Value;
}

void Location::del(const std::string & Key) {
    std::map<std::string, std::string>::iterator it = location.find(Key);
	if (it != location.end()) location.erase(it);
}

//	Clear

void Location::clear() { location.clear(); }
