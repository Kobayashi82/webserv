/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 11:59:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 15:49:00 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Colors.hpp"

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <sstream>																						//	For std::ostringstream to format strings
#include <iomanip>																						//	For stream manipulators like std::setw and std::setfill
#include <csignal>																						//	For signal handling functions like std::signal

#include <fcntl.h>																						//	For file control options like fcntl to set non-blocking mode
#include <termios.h>																					//	For terminal I/O interfaces to enable/disable raw mode
#include <sys/ioctl.h>																					//	For terminal control functions like ioctl to get window size

class Display {

	public:

		//	Methods
		static void	Input();																			//	Manage user input
		static void	Output();																			//	Manage output to the terminal
		static void	enableRawMode();																	//	Enable raw un-buffered mode for the terminal
		static void	disableRawMode();																	//	Disable raw un-buffered mode for the terminal
		static void	setTerminalSize(size_t rows, size_t cols);											//	Set the terminal size (a little big buggy)

};
