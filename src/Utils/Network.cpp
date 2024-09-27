/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Network.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 13:59:39 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/27 23:18:06 by vzurera-         ###   ########.fr       */
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
			if ((*it).length() > 0 && !std::isdigit((*it)[0])) return (false);
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
		if (mask.find('.') != std::string::npos && !isValidMask(mask))				return (false);
		else { long number; if (stol(mask, number) || number < 0 || number > 32) 	return (false); }

		return (true);
	}

#pragma endregion

#pragma region Mask Valid

	bool Utils::isValidMask(const std::string & mask) {
		if (mask.empty()) return (false);

		if (mask.find('.') != std::string::npos) {
			const char * maskArray[] = {
				"255.255.255.255", "255.255.255.254", "255.255.255.252", "255.255.255.248",
				"255.255.255.240", "255.255.255.224", "255.255.255.192", "255.255.255.128",
				"255.255.255.0", "255.255.254.0", "255.255.252.0", "255.255.248.0",
				"255.255.240.0", "255.255.224.0", "255.255.192.0", "255.255.128.0",
				"255.255.0.0", "255.254.0.0", "255.252.0.0", "255.248.0.0",
				"255.240.0.0", "255.224.0.0", "255.192.0.0", "255.128.0.0",
				"255.0.0.0", "254.0.0.0", "252.0.0.0", "248.0.0.0",
				"240.0.0.0", "224.0.0.0", "192.0.0.0", "128.0.0.0", "0.0.0.0"
			};

			std::vector<std::string> validMasks(maskArray, maskArray + sizeof(maskArray) / sizeof(maskArray[0]));
			return (std::find(validMasks.begin(), validMasks.end(), mask) != validMasks.end());

		} else { long number; if (stol(mask, number) || number < 0 || number > 32) 	return (false); }

		return (true);
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
		if (range.size() > 6 && range.substr(0, 7) == "0.0.0.0") return (true);
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

#pragma region Add Address

	std::vector <std::string> get_ip_range(const std::string & IP, const std::string & mask) {
		std::vector<std::string> ip_range;
		unsigned int ip_addr = ipToInt(IP);
		unsigned int mask_addr = ipToInt(mask);
		unsigned int network_addr = ip_addr & mask_addr;
		unsigned int broadcast_addr = network_addr | ~mask_addr;

		for (unsigned int addr = network_addr; addr <= broadcast_addr; ++addr) {
			std::ostringstream oss; oss << ((addr >> 24) & 0xFF) << "." << ((addr >> 16) & 0xFF) << "." << ((addr >> 8) & 0xFF) << "." << (addr & 0xFF);
			ip_range.push_back(oss.str());
		}

		return (ip_range);
	}

	void Utils::add_address(const std::string & IP, long port, VServer & VServ) {
		if (IP == "0.0.0.0") { VServ.addresses.insert(VServ.addresses.begin(), (std::make_pair(IP, port))); return; }
		std::string ip = IP; std::string mask = "255.255.255.255";

		size_t slash_pos = IP.find('/');
		if (slash_pos != std::string::npos) {
			ip = IP.substr(0, slash_pos);
			std::string mask_str = IP.substr(slash_pos + 1);
			if (mask_str.find('.') != std::string::npos) mask = mask_str;
			else {
				long cidr; Utils::stol(mask_str, cidr); unsigned int mask_num = (0xFFFFFFFF << (32 - cidr)) & 0xFFFFFFFF;
				std::ostringstream oss; oss << ((mask_num >> 24) & 0xFF) << "." << ((mask_num >> 16) & 0xFF) << "." << ((mask_num >> 8) & 0xFF) << "." << (mask_num & 0xFF);
				mask = oss.str();
			}
		}

		std::vector<std::string> ip_range = get_ip_range(ip, mask);

    	std::string first_ip = ip_range.front();
    	std::string broadcast_ip = ip_range.back();

		for (std::vector<std::string>::const_iterator it = ip_range.begin(); it != ip_range.end(); ++it) {
			if (ip_range.size() > 2 && (*it == first_ip || *it == broadcast_ip)) {
            	continue; // Skip the first and broadcast addresses
        	}
			bool exists = false;
	
			for (std::vector <std::pair<std::string, int> >::const_iterator addr_it = VServ.addresses.begin(); addr_it != VServ.addresses.end(); ++addr_it)
				if (addr_it->first == *it && addr_it->second == port) { exists = true; break; }

			if (!exists) VServ.addresses.push_back(std::make_pair(*it, port));
		}
	}

#pragma endregion

#pragma region Valid URL

	bool Utils::valid_url(const std::string & url) {
		const std::string blacklist = ";|&`><$\\!";

		for (size_t i = 0; i < url.size(); ++i) {
			char ch = url[i];
			if (blacklist.find(ch) != std::string::npos) return (false);
		}

		if (url.find("../") != std::string::npos || url.find("..\\") != std::string::npos) return (false);

		return (true);
	}

#pragma endregion
