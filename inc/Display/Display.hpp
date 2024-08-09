/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 11:59:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/09 23:13:22 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Colors.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <sstream>																						//	For std::stringstream to format strings
#include <iomanip>																						//	For stream manipulators like std::setw and std::setfill
#include <csignal>																						//	For signal handling like std::signal

#include <fcntl.h>																						//	For file control options like fcntl to set non-blocking mode
#include <termios.h>																					//	For terminal I/O interfaces to enable/disable raw mode
#include <sys/ioctl.h>																					//	For terminal control functions like ioctl to get window size

class Display {

	public:

		static bool	drawing;
		static int	failCount;
		static int	maxFails;

		//	Methods
		static void	Input();																			//	Manage user input
		static void	Output();																			//	Manage output to the terminal
		static void	enableRawMode();																	//	Enable raw un-buffered mode for the terminal
		static void	disableRawMode();																	//	Disable raw un-buffered mode for the terminal
		static void	setTerminalSize(size_t rows, size_t cols);											//	Set the terminal size (a little big buggy)

};
