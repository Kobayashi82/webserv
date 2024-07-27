/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:54:43 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/27 16:39:01 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VServer.hpp"

//	VServer

std::string VServer::get(const std::string & Key) const {
    std::map<std::string, std::string>::const_iterator it = vserver.find(Key);
    if (it == vserver.end()) return ("");
	return (it->second);
}

void VServer::set(const std::string & Key, const std::string & Value) {
    std::map<std::string, std::string>::iterator it = vserver.find(Key);
	if (it != vserver.end()) it->second = Value;
	if (it == vserver.end()) vserver[Key] = Value;
}

void VServer::del(const std::string & Key) {
    std::map<std::string, std::string>::iterator it = vserver.find(Key);
	if (it != vserver.end()) vserver.erase(it);
}


//	Location

void VServer::set(const Location & Loc) {
    std::vector<Location>::iterator it = std::find(location.begin(), location.end(), Loc);
    if (it == location.end()) location.push_back(Loc);
}

void VServer::del(const Location & Loc) {
    std::vector<Location>::iterator it = std::find(location.begin(), location.end(), Loc);
    if (it != location.end()) { it->clear(); location.erase(it); }
}


//	Clear

void VServer::clear() {
	vserver.clear();
	for (std::vector<Location>::iterator it = location.begin(); it != location.end(); ++it) it->clear();
}
