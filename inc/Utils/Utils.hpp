/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 21:37:20 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/24 17:50:41 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "VServer.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout

class Utils {

	public:

		//	String
		static void 		trim(std::string & str);													//	Trim a string passed as reference from spaces in both sides and also trim any "# comments"
		static void 		toLower(std::string & str);													//	Convert a string passed as reference to lower case
		static void			toUpper(std::string & str);													//	Convert a string passed as reference to upper case
		static bool			stol(const std::string & str, long & number, bool ignore_eof = false);		//	Convert a string to a number passed as reference (return 0 if success)
		static std::string	ltos(long number);															//	Convert a long number to string
		static std::string	dtos(double number);														//	Convert a double number to string
		static std::string	formatSize(size_t bytes, bool just_suffix = false);							//	Format a size to string (byte, KB, MB, GB, TB)
		static void			formatSize(size_t bytes, std::string & data1, std::string & data2);			//	Format a size to string (byte, KB, MB, GB, TB) and set it to data1 and data2
		static double		formatSizeDbl(size_t bytes);												//	Format a size to double (byte, KB, MB, GB, TB)
		static int			str_nocolor_length(const std::string & str);								//	Get the length of a string without the colors chars
		static std::string	str_nocolor_trunc(const std::string & str, int length);						//	Get a string truncated to the length without counting colors chars
		static std::string	str_nocolor(const std::string & str);										//	Get a string without the colors chars
		static std::string	replace_tabs(const std::string & str, int tabSize = 8);

		//	Network
		static bool			isValidIP(const std::string & IP);											//	Validate an IP
		static bool			isValidMask(const std::string & mask);										//	Validate a mask
		static bool			isIPInRange(const std::string & IP, const std::string & range);				//	Check if an IP is in a given range
		static void			add_address(const std::string & IP, long port, VServer & VServ);
		static bool			isValidPort(std::string port);												//	Validate a Port in string format
		static bool			isValidPort(int port);														//	Validate a Port in numeric format

		//	Files
		static std::string	programPath();																//	Get the path of the executable (return '/' if fails)
		static int 			createPath(const std::string & path);										//	Create the indicated path (return 0 if success)
		static int 			file_exists(const std::string & File);										//	Check if a file exists (0 = Exists, 1 = Don't Exists, 2 = No read permissions)
		static bool			isFile(const std::string & path);											//	Check if a Path is a file
		static bool			isDirectory(const std::string & path);										//	Check if a Path is a directory

};
