/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 11:54:43 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 19:56:59 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VServer.hpp"
#include "Settings.hpp"

#pragma region Constructors

    VServer::VServer() : config_displayed(false), config_index(0), log_index(0), autolog(true), status(false), force_off(false), bad_config(false) {}
    VServer::VServer(const VServer & src) { *this = src; }
    VServer::~VServer() { clear(); }

#pragma endregion

#pragma region Overloads

	VServer &	VServer::operator=(const VServer & rhs) {
        if (this != &rhs) {
            data = rhs.data; addresses = rhs.addresses; location = rhs.location; method = rhs.method; log = rhs.log; config = rhs.config; bad_config = rhs.bad_config; log = rhs.log;
            config_displayed = rhs.config_displayed; config_index = rhs.config_index; log_index = rhs.log_index; status = rhs.status; force_off = rhs.force_off; autolog = rhs.autolog;
        } return (*this);
    }

	bool		VServer::operator==(const VServer & rhs) const { return (data == rhs.data && addresses == rhs.addresses && location == rhs.location && method == rhs.method); }

#pragma endregion

#pragma region VServer

    #pragma region Get

        std::string VServer::get(const std::string & Key) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
				if (it->first == Key) return (it->second);

			return ("");
        }

    #pragma endregion

    #pragma region Set/Add

        void VServer::set(const std::string & Key, const std::string & Value, bool Force) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
				if (!Force && it->first == Key) { it->second = Value; return; }

			data.push_back(std::make_pair(Key, Value));
        }

		void VServer::add(const std::string & Key, const std::string & Value, bool Force) { set(Key, Value, Force); }

    #pragma endregion

    #pragma region Del

        void VServer::del(const std::string & Key) {
			for (std::vector <std::pair<std::string, std::string> >::iterator it = data.begin(); it != data.end(); ++it)
				if (it->first == Key) { data.erase(it); }
        }

    #pragma endregion

    #pragma region Clear

        void VServer::clear() {
            for (std::deque <Location>::iterator it = location.begin(); it != location.end(); ++it) it->clear();
            data.clear(); addresses.clear(); location.clear(); method.clear(); config.clear(); log.clear();
        }

        void VServer::clear_logs() { log.clear(); }

    #pragma endregion

#pragma endregion

#pragma region Location

    #pragma region Set/Add

        void VServer::set(const Location & Loc) {
            std::deque <Location>::iterator it = std::find(location.begin(), location.end(), Loc);
            if (it == location.end()) location.push_back(Loc);
        }

		void VServer::add(const Location & Loc) { set(Loc); }

    #pragma endregion

    #pragma region Del

        void VServer::del(const Location & Loc) {
            std::deque <Location>::iterator it = std::find(location.begin(), location.end(), Loc);
        		if (it != location.end()) { it->clear(); location.erase(it); }
        }

    #pragma endregion

#pragma endregion

#pragma region Method

    #pragma region Set/Add

        void VServer::set(const Method & Met) {
            std::deque <Method>::iterator it = std::find(method.begin(), method.end(), Met);
            if (it == method.end()) method.push_back(Met);
        }

		void VServer::add(const Method & Met) { set(Met); }

    #pragma endregion

    #pragma region Del

        void VServer::del(const Method & Met) {
            std::deque <Method>::iterator it = std::find(method.begin(), method.end(), Met);
            if (it != method.end()) { it->clear(); method.erase(it); }
        }

    #pragma endregion

#pragma endregion

#pragma region Addresses

    #pragma region Set/Add

    	void VServer::set_address(const std::string & IP, const int & Port) { addresses.push_back(std::make_pair(IP, Port)); }

		void VServer::add_address(const std::string & IP, const int & Port) { set_address(IP, Port); }

    #pragma endregion

    #pragma region Del

        void VServer::del_address(const std::string & IP, const int & Port) {
            for (std::vector <std::pair<std::string, int> >::iterator it = addresses.begin(); it != addresses.end(); ++it)
            	if (it->first == IP && it->second == Port) addresses.erase(it);
        }

    #pragma endregion

#pragma endregion
