/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 21:37:20 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/28 22:31:59 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "VServer.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout

#pragma region Utils

class Utils {

	public:

		//	String
		static void 		trim(std::string & str);													//	Trim a string passed by reference from spaces on both sides and also trim any "# comments"
		static std::string	strim(std::string str);														//	Trim a string passed by reference from spaces on both sides and also trim any "# comments"
		static std::string	escape_spaces(const std::string & str);										//	Escape spaces in the string passed by reference and return a new string
		static void 		toLower(std::string & str);													//	Convert a string passed by reference to lower case
		static void			toUpper(std::string & str);													//	Convert a string passed by reference to upper case
		static std::string	strToLower(const std::string & str);										//	Convert a string passed by reference to lower case
		static std::string	strToUpper(const std::string & str);										//	Convert a string passed by reference to upper case
		static bool			stol(const std::string & str, long & number, bool ignore_eof = false);		//	Convert a string passed by reference to a long number 	(return 0 if success)
		static bool			stol(const std::string & str, size_t & number, bool ignore_eof = false);	//	Convert a string passed by reference to a size_t number	(return 0 if success)
		static long			sstol(const std::string & str);												//	Convert a string passed by reference to a long number
		static std::string	ltos(long number);															//	Convert a long number to a string
		static std::string	dtos(double number);														//	Convert a double number to a string
		static std::string	formatSize(size_t bytes, bool just_suffix = false, int decimals = 2);		//	Format a size as a string (byte, KB, MB, GB, TB)
		static void			formatSize(size_t bytes, std::string & data1, std::string & data2);			//	Format a size as a string (byte, KB, MB, GB, TB) and set it to data1 and data2
		static double		formatSizeDbl(size_t bytes);												//	Format a size as a double (byte, KB, MB, GB, TB)
		static int			str_nocolor_length(const std::string & str);								//	Get the length of a string without color characters
		static std::string	str_nocolor_trunc(const std::string & str, int length);						//	Get a string truncated to the length without counting color characters
		static std::string	str_nocolor(const std::string & str);										//	Get a string without color characters
		static std::string	replace_tabs(const std::string & str, int tabSize = 8);						//	Replace tabs with spaces
		static std::string	str_time(time_t time);														//	Get the time in seconds as a string

		//	Network
		static bool			isValidIP(const std::string & IP);											//	Validate an IP address
		static bool			isValidMask(const std::string & mask);										//	Validate a mask
		static bool			isIPInRange(const std::string & IP, const std::string & range);				//	Check if an IP address is within a given range
		static void			add_address(const std::string & IP, long port, VServer & VServ);			//	Add an IP address to the list of IP addresses of a VServer
		static bool			isValidPort(std::string port);												//	Validate a port in string format
		static bool			isValidPort(int port);														//	Validate a port in numeric format
		static bool 		valid_url(const std::string & url);											//	Validate a URL or resource path

		//	Files
		static std::string	programPath();																//	Get the path of the executable (return '/' if fails)
		static int 			createPath(const std::string & path);										//	Create the indicated path (return 0 if successful)
		static int 			file_exists(const std::string & path, bool is_exec = false);				//	Check if a file exists (0 = Exists, 1 = Does Not Exist, 2 = No read/execution permissions, 3 = Not a file)
		static int 			directory_exists(const std::string & path);									//	Check if a directory exists (0 = Exists, 1 = Does Not Exist, 2 = No read permissions, 3 = Not a directory)
		static bool			isFile(const std::string & path);											//	Check if a path is a file
		static bool			isDirectory(const std::string & path);										//	Check if a path is a directory
		static std::string	fullpath(const std::string & path);											//	Return the full path of a path (process './' and '../' and must be absolute)
		static bool 		is_subpath(const std::string & path1, const std::string & path2);			//	Check if path1 is inside path2
		static size_t 		filesize(const std::string & path);											//	Get the size of a file from a path
		static size_t 		filesize(const int fd);														//	Get the size of a file from a file descriptor (FD)
		static std::string	file_modification_time(const std::string path, bool is_header = false);		//	Get the modification time of a file
		static time_t		file_modification_time_data(const std::string & path);						//	Get the modification time of a file as number of seconds
		static std::string	expand_tilde(const std::string & path);										//	Expand '~' to the user home directory
		static void			NonBlocking_FD(int fd);														//	Set a file descriptor (FD) as Non-Blocking
		static std::string	get_OSname();																//	Get the name of the operating system

};

#pragma endregion
