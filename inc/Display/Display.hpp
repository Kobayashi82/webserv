/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 11:59:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/21 11:57:03 by vzurera-         ###   ########.fr       */
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

		static bool	drawing;																			//	True if printing in the terminal
		static int	failCount;																			//	Current number of fails when printing in the the terminal
		static int	maxFails;																			//	Maximum numbers of retries to print in the terminal if something fails
		static bool	RawModeDisabled;																	//	Status of the terminal (false if in raw mode)
		static bool	ForceRawModeDisabled;																//	Force terminal in normal mode (not raw mode)
		static bool	Resized;																			//	True if the terminal has been resized
		static bool	redraw;																				//	Is in the middle of an Output()
		static bool	terminate;																			//	Flag the thread to finish

		//	Methods
		static void	Logo();																				//	Print the Webserv logo
		static void	Input();																			//	Manage user input
		static void	Output();																			//	Manage output to the terminal
		static void	enableRawMode();																	//	Enable raw un-buffered mode for the terminal
		static void	disableRawMode();																	//	Disable raw un-buffered mode for the terminal
		static void	setTerminalSize(size_t rows, size_t cols);											//	Set the terminal size (a little big buggy)

		static void	* main_display(void * args);

	private:

		static const int 							TERMINAL_INTERVAL;									//	Interval in miliseconds between updates for the terminal display

};
