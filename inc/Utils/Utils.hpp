/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 21:37:20 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 23:56:35 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout

class Utils {

	public:

		//	String
		static void 		trim(std::string & str);													//	Trim a string passed as reference from spaces in both sides and also trim any "# comments"
		static void 		toLower(std::string & str);													//	Convert a string passed as reference to lower case
		static void			toUpper(std::string & str);													//	Convert a string passed as reference to upper case
		static long			stol(const std::string & str, long & number);								//	Convert a string to a number passed as reference (return 0 if success)
		static std::string	ltos(long number);															//	Convert a long number to string
		static std::string	stod(double number);														//	Convert a double number to string
		static std::string	formatSize(size_t bytes);													//	Format a size to string (byte, KB, MB, GB, TB)

		//	Files
		static std::string	programPath();																//	Get the path of the executable (return '/' if fails)
		static int 			createPath(const std::string & path);										//	Create the indicated path (return 0 if success)
		static int 			file_exists(const std::string & File);										//	Check if a file exists (0 = Exists, 1 = Don't Exists, 2 = No read permissions)

};
