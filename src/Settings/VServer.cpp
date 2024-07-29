/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:54:43 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/29 20:57:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VServer.hpp"

#pragma region VServer

    #pragma region Get

        std::string VServer::get(const std::string & Key) const {
            std::map<std::string, std::string>::const_iterator it = vserver.find(Key);
            if (it == vserver.end()) return ("");
            return (it->second);
        }

    #pragma endregion

    #pragma region Set

        void VServer::set(const std::string & Key, const std::string & Value) {
            std::map<std::string, std::string>::iterator it = vserver.find(Key);
            if (it != vserver.end()) it->second = Value;
            if (it == vserver.end()) vserver[Key] = Value;
        }

    #pragma endregion

    #pragma region Del

        void VServer::del(const std::string & Key) {
            std::map<std::string, std::string>::iterator it = vserver.find(Key);
            if (it != vserver.end()) vserver.erase(it);
        }

    #pragma endregion

    #pragma region Clear

        void VServer::clear() {
            for (std::vector<Location>::iterator it = location.begin(); it != location.end(); ++it) it->clear();
            vserver.clear(); location.clear();
        }

    #pragma endregion

#pragma endregion

#pragma region Location

    #pragma region Set

        void VServer::set(const Location & Loc) {
            std::vector<Location>::iterator it = std::find(location.begin(), location.end(), Loc);
            if (it == location.end()) location.push_back(Loc);
        }

    #pragma endregion

    #pragma region Del

        void VServer::del(const Location & Loc) {
            std::vector<Location>::iterator it = std::find(location.begin(), location.end(), Loc);
            if (it != location.end()) { it->clear(); location.erase(it); }
        }

    #pragma endregion

#pragma endregion
