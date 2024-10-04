/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   String.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 21:36:49 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/04 15:41:08 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

#include <sstream>																						//	For std::stringstream to format strings
#include <iomanip>																						//	For stream manipulators like std::setw and std::setfill
#include <cstdio>																						//	For sprintf() to get a time_t in seconds
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

	std::string Utils::strim(std::string str) {
		if (str.empty()) return (str);
		std::string::iterator start = str.begin();
		std::string::iterator end = str.end();

		while (start != str.end() && std::isspace(static_cast<unsigned char>(*start))) start++;

		std::string::iterator hashPos = std::find(start, str.end(), '#');
		if (hashPos != str.end()) end = hashPos;

		while (end != start && std::isspace(static_cast<unsigned char>(*(end - 1)))) --end;
		str = std::string(start, end);

		return (str);
	}

#pragma endregion

#pragma region Escape Spaces

	std::string Utils::escape_spaces(const std::string & str) {
		std::string result;

		for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
			if (*it == ' ') result += "\\ "; else result += *it;

		return (result);
	}

#pragma endregion

#pragma region toLower / toUpper

	void Utils::toLower(std::string & str) { for (size_t i = 0; i < str.size(); ++i) str[i] = std::tolower(static_cast<unsigned char>(str[i])); }
	void Utils::toUpper(std::string & str) { for (size_t i = 0; i < str.size(); ++i) str[i] = std::toupper(static_cast<unsigned char>(str[i])); }

	std::string Utils::strToLower(const std::string & str) {
		std::string result = str;
		for (size_t i = 0; i < result.size(); ++i) result[i] = std::tolower(static_cast<unsigned char>(result[i]));
		return (result);
	}
	std::string Utils::strToUpper(const std::string & str) {
		std::string result = str;
		for (size_t i = 0; i < result.size(); ++i) result[i] = std::toupper(static_cast<unsigned char>(result[i]));
		return (result);
	}

#pragma endregion

#pragma region ltos

	std::string Utils::ltos(long number) {
		std::stringstream ss; ss << number;
		if (ss.fail()) return ("");
		return (ss.str());
	}

#pragma endregion

#pragma region stol

	bool Utils::stol(const std::string & str, long & number, bool ignore_eof) {
		if (str.empty()) return (true);
		std::stringstream ss(str); ss >> number;
		return ((ss.fail() || (!ignore_eof && !ss.eof())));
	}

	bool Utils::stol(const std::string & str, size_t & number, bool ignore_eof) {
		if (str.empty()) return (true);
		std::stringstream ss(str); ss >> number;
		return ((ss.fail() || (!ignore_eof && !ss.eof())));
	}

	long Utils::sstol(const std::string & str) {
		if (str.empty()) return (0);

		long number;
		std::stringstream ss(str); ss >> number;

		return (number);
	}

#pragma endregion

#pragma region dtos

	std::string Utils::dtos(double number) {
		std::ostringstream oss; oss << std::fixed << std::setprecision(2) << number; std::string Result = oss.str();		// Convert to string with sufficient precision

		size_t decimalPos = Result.find('.');
		if (decimalPos != std::string::npos) {
			size_t End = Result.find_last_not_of('0');																		// Remove trailing zeros after the decimal point
			if (End != std::string::npos && End > decimalPos) Result.erase(End + 1); else Result.erase(decimalPos);			// Remove decimal point if there are only zeros after it
		} return (Result);
	}

#pragma endregion

#pragma region Format Size

	std::string Utils::formatSize(size_t bytes, bool just_suffix, int decimals) {
		const char * suffixes[] = {"KB", "MB", "GB", "TB"};;
		size_t suffix = 0; double size = static_cast<double>(bytes) / 1024.0;

		while (size >= 1024 && suffix < 3) { size /= 1024; ++suffix; }
		if (just_suffix)		return (suffixes[suffix]);
		
		std::ostringstream oss; oss << std::fixed << std::setprecision(decimals) << size;
		return (oss.str() + " " + suffixes[suffix]);
	}

	void Utils::formatSize(size_t bytes, std::string & data1, std::string & data2) {
		const char * suffixes[] = {"KB", "MB", "GB", "TB"};
		size_t suffix = 0; double size = static_cast<double>(bytes) / 1024.0;

		while (size >= 1024 && suffix < 3) { size /= 1024; ++suffix; }

		data1 = dtos(size);
		data2 = suffixes[suffix];
	}

	double Utils::formatSizeDbl(size_t bytes) {
		size_t suffix = 0; double size = static_cast<double>(bytes) / 1024.0;

		while (size >= 1024 && suffix < 3) { size /= 1024; ++suffix; }

		return (size);
	}

#pragma endregion

#pragma region STR NoColor

	std::string Utils::str_nocolor(const std::string & str) {
		std::string result; int length = str.size();
		int visible_length = 0;
		for (size_t i = 0; i < str.size(); ++i) {
			if (str[i] == '\033' && i + 1 < str.size() && str[i + 1] == '[') {
				while (i < str.size() && str[i] != 'm') ++i;
			} else {
				if (visible_length < length) {
					result += str[i];
					++visible_length;
				} else {
					result += "...";
					break;
				}
			}
		}
		return result;
	}

#pragma endregion

#pragma region STR NoColor Trunc

	static std::string replace_tabs_with_spaces(const std::string & str, int spaces_per_tab = 8) {
		std::string result;
		for (size_t i = 0; i < str.size(); ++i) {
			if (str[i] == '\t') { for (int j = 0; j < spaces_per_tab; ++j) result += ' ';}
			else result += str[i];
		}
		return (result);
	}

	// std::string Utils::str_nocolor_trunc(const std::string & str, int length) {
	// 	std::string result; int visible_length = 0;
	// 	for (size_t i = 0; i < str.size(); ++i) {
	// 		if (str[i] == '\033' && i + 1 < str.size() && str[i + 1] == '[') {
	// 			while (i < str.size() && str[i] != 'm') result += str[i++];
	// 			result += str[i];
	// 		} else if (str[i] == '\t') {
	// 			if (visible_length + 8 < length) { result += str[i]; visible_length+= 8; }
	// 			else { result += "..."; break; }
	// 		} else {
	// 			if (visible_length < length) { result += str[i]; ++visible_length; }
	// 			else { result += "..."; break; }
	// 		}
	// 	}
	// 	return (result);
	// }

	std::string Utils::str_nocolor_trunc(const std::string & str, int length) {
		std::string result; int visible_length = 0;
		std::string temp = replace_tabs_with_spaces(str);
		for (size_t i = 0; i < temp.size(); ++i) {
			if (temp[i] == '\033' && i + 1 < temp.size() && temp[i + 1] == '[') {
				while (i < temp.size() && temp[i] != 'm') result += temp[i++];
				result += temp[i];
			} else {
				if (visible_length < length) { result += temp[i]; ++visible_length; }
				else { result += "..."; break; }
			}
		}
		return (result);
	}

#pragma endregion

#pragma region STR NoColor Length

	int Utils::str_nocolor_length(const std::string & str) {
		std::string temp = replace_tabs_with_spaces(str);
	    int length = 0;
		for (size_t i = 0; i < temp.size(); ++i) {
			if (temp[i] == '\033' && i + 1 < temp.size() && temp[i + 1] == '[')
				while (i < temp.size() && temp[i] != 'm') ++i;
			else ++length;
		}
		return (length);
	}

#pragma endregion

#pragma region Replace Tabs

	std::string Utils::replace_tabs(const std::string & str, int tabSize) {
		std::string result;

		for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
			if (*it == '\t')	result.append(tabSize, ' ');
			else				result += *it;
		}
		return (result);
	}

#pragma endregion

#pragma region STR Time

	std::string Utils::str_time(time_t time) {
		char buffer[20];

		sprintf(buffer, "%ld", (long)time);

		return (std::string(buffer));
	}

#pragma endregion

#pragma region Line Spaces On

	std::string Utils::line_spaces_on(const std::string & str) {
		std::string tmp;

		for (size_t i = 0; i < str.size(); ++i) {
			if		(str[i] == '\\' && str[i + 1] == ' ') {	tmp += " "; i++; }
			else if	(str[i] == '\\' && str[i + 1] == '_') {	tmp += " "; i++; }
			else											tmp += str[i];
		}

		return (tmp);
	}

#pragma endregion

#pragma region Line Spaces Off

	std::string Utils::line_spaces_off(const std::string & str) {
		std::string tmp;

		for (size_t i = 0; i < str.size(); ++i) {
			if (str[i] == '\\' && str[i + 1] == ' ') {	tmp += "\\_"; i++; }
			else										tmp += str[i];
		}

		return (tmp);
	}

#pragma endregion
