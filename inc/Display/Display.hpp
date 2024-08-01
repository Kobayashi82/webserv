/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 11:59:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/01 23:33:06 by vzurera-         ###   ########.fr       */
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

		static int	cols;
		static int	rows;
		static int	log_rows;

		static void	Input();
		static void	Output();
		static void	enableRawMode();
		static void	disableRawMode();
		static void	setTerminalSize(size_t rows, size_t cols);

};