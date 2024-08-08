/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   String.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 21:36:49 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 23:11:12 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

#include <sstream>																						//	For std::stringstream to format strings
#include <iomanip>																						//	For stream manipulators like std::setw and std::setfill
#include <algorithm>																					//	For std::find()

#pragma region Trim

	void Utils::trim(std::string & str) {
		if (str.empty()) return;
		std::string::iterator start = str.begin();
		std::string::iterator end = str.end();

		while (start != str.end() && std::isspace(static_cast<unsigned char>(*start))) start++;

		std::string::iterator hashPos = std::find(start, str.end(), '#');
		if (hashPos != str.end()) end = hashPos;

		while (end != start && std::isspace(static_cast<unsigned char>(*(end - 1)))) --end;
		str = std::string(start, end);
	}

#pragma endregion

#pragma region toLower / toUpper

	void Utils::toLower(std::string & str) { for (size_t i = 0; i < str.size(); ++i) str[i] = std::tolower(static_cast<unsigned char>(str[i])); }
	void Utils::toUpper(std::string & str) { for (size_t i = 0; i < str.size(); ++i) str[i] = std::toupper(static_cast<unsigned char>(str[i])); }

#pragma endregion

#pragma region ltos

	std::string Utils::ltos(long number) {
		std::stringstream ss; ss << number;
		if (ss.fail()) return ("");
		return (ss.str());
	}

#pragma endregion

#pragma region stol

	long Utils::stol(const std::string & str, long & number) {
		std::stringstream ss(str);
		ss >> number; return ((ss.fail() || !ss.eof()));
	}

#pragma endregion

#pragma region stod

	std::string Utils::stod(double number) {
		std::ostringstream oss; oss << std::fixed << std::setprecision(2) << number; std::string Result = oss.str();		// Convert to string with sufficient precision

		size_t decimalPos = Result.find('.');
		if (decimalPos != std::string::npos) {
			size_t End = Result.find_last_not_of('0');																		// Remove trailing zeros after the decimal point
			if (End != std::string::npos && End > decimalPos) Result.erase(End + 1); else Result.erase(decimalPos);			// Remove decimal point if there are only zeros after it
		} return (Result);
	}

#pragma endregion

#pragma region Format Size

	std::string Utils::formatSize(size_t bytes) {
		const char * suffixes[] = {"byte", "KB", "MB", "GB", "TB"}; std::string s = "";
		size_t suffix = 0; double size = static_cast<double>(bytes);

		while (size >= 1024 && suffix < 4) { size /= 1024; ++suffix; }
		if (size > 1 && suffix == 0) s = "s";
		return (stod(size) + " " + suffixes[suffix] + s);
	}

#pragma endregion
