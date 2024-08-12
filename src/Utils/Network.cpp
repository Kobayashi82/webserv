/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Network.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 13:59:39 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/12 16:25:36 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

#include <sstream>																						//	For std::stringstream to format strings
#include <vector>																						//	For std::vector container
#include <algorithm>																					//	For std::find()

#pragma region IP Valid

	static bool isValid_IPAddress(const std::string & IP) {												//	Validate an IP
		std::vector <std::string> segments; std::string segment;
		std::string::size_type start = 0; std::string::size_type end = IP.find('.');

		while (end != std::string::npos) {
			segment = IP.substr(start, end - start); segments.push_back(segment);
			start = end + 1; end = IP.find('.', start);
		} segments.push_back(IP.substr(start));
		if (segments.size() != 4) return (false);

		for (std::vector <std::string>::const_iterator it = segments.begin(); it != segments.end(); ++it) {
			if (static_cast<std::string>(*it).empty()) return (false);
			if ((*it).length() > 1 && (*it)[0] == '0') return (false);
			long number; if (Utils::stol(*it, number) || number < 0 || number > 255) return (false);
		}
		return (true);
	}

	bool Utils::isValidIP(const std::string & IP) {														//	Validate an IP or a CIDR
		std::string::size_type slashPos = IP.find('/');
		if (slashPos == std::string::npos) return (isValid_IPAddress(IP));

		std::string ip = IP.substr(0, slashPos);
		std::string mask = IP.substr(slashPos + 1);

		if (!isValid_IPAddress(ip)) return (false);
		if (mask.find('.') != std::string::npos) return (isValid_IPAddress(mask));
		long number; if (stol(mask, number) || number < 0 || number > 32) return (false);
		return (true);
	}

#pragma endregion

#pragma region IP in Range

	static unsigned int ipToInt(const std::string & IP) {												//	Convert an IP to integer
		std::vector <std::string> segments; std::string segment;
		std::string::size_type start = 0; std::string::size_type end = IP.find('.');

		while (end != std::string::npos) {
			segment = IP.substr(start, end - start); segments.push_back(segment);
			start = end + 1; end = IP.find('.', start);
		} segments.push_back(IP.substr(start));

		unsigned int result = 0;
		for (std::vector <std::string>::const_iterator it = segments.begin(); it != segments.end(); ++it)
			result = (result << 8) + std::atoi(it->c_str());
		return (result);
	}

	bool Utils::isIPInRange(const std::string & IP, const std::string & range) {						//	Check if an IP is in a given range
		std::string::size_type slashPos = range.find('/');
		if (!isValid_IPAddress(IP)) return (false);
		if (slashPos == std::string::npos) return (IP == range);

		if (!isValidIP(range)) return (false);
		std::string ip = range.substr(0, slashPos);
		std::string mask = range.substr(slashPos + 1);

		unsigned int ipInt = ipToInt(IP);
		unsigned int baseIPInt = ipToInt(ip);
		unsigned int maskInt = 0;

		if (mask.empty())								maskInt = 0xFFFFFFFF;
		else if (mask.find('.') != std::string::npos)	maskInt = ipToInt(mask);
		else { int cidr = std::atoi(mask.c_str());
			if (cidr == 0)								maskInt = 0;
			else 										maskInt = 0xFFFFFFFF << (32 - cidr);
		}
		return ((ipInt & maskInt) == (baseIPInt & maskInt));
	}

#pragma endregion

#pragma region Port Valid

	bool Utils::isValidPort(std::string port) {
		std::string temp; std::istringstream stream(port); stream >> temp;
		
		if (temp.empty()) return (false);
		long number; if (Utils::stol(temp, number) || number < 1 || number > 65535) return (false);
		return (true);
	}

	bool Utils::isValidPort(int port) { return (port > 0 && port <= 65535); }

#pragma endregion
