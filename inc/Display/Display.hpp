/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 11:59:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 15:34:19 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Colors.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <sstream>																						//	For std::stringstream to format strings
#include <iomanip>																						//	For stream manipulators like std::setw and std::setfill
#include <csignal>																						//	For signal handling like std::signal

#include <pthread.h>																					//	For multi-threading and synchronization
#include <fcntl.h>																						//	For file control options like fcntl to set non-blocking mode
#include <termios.h>																					//	For terminal I/O interfaces to enable/disable raw mode
#include <sys/ioctl.h>																					//	For terminal control functions like ioctl to get window size

#pragma region Display

class Display {

	public:

		static pthread_mutex_t	mutex;																	//	Mutex for synchronizing access to shared resources

		static bool				drawing;																//	True if printing in the terminal
		static int				failCount;																//	Current number of fails when printing in the the terminal
		static int				maxFails;																//	Maximum numbers of retries to print in the terminal if something fails
		static bool				RawModeDisabled;														//	Status of the terminal (false if in raw mode)
		static bool				ForceRawModeDisabled;													//	Force terminal in normal mode (not raw mode)
		static bool				Resized;																//	True if the terminal has been resized
		static bool				redraw;																	//	Is in the middle of an Output()
		static bool				background;																//	True if the program is running in background (&)
		static int				signal;																	//	Last signal code


		//	Methods
		static void	signal_handler();																	//	Handle signals
		static int	signal_check();																		//	Check for signals to process
		static void	Logo();																				//	Print the Webserv logo
		static void	Input();																			//	Manage user input
		static void	Output();																			//	Manage output to the terminal
		static void	enableRawMode();																	//	Enable raw un-buffered mode for the terminal
		static void	disableRawMode();																	//	Disable raw un-buffered mode for the terminal
		static void	setTerminalSize(size_t rows, size_t cols);											//	Set the terminal size (a little big buggy)

		static void	update();																			//	Ask for a redraw in the next iteration
		static void	logo();																				//	Ask for a logo print in the next iteration
		static int	isTerminate();																		//	Checks if the main termination flag is set
		static bool	isRawMode();																		//	Checks if the program is in raw mode

		static void	start();																			//	Start the thread
		static void	stop();																				//	Stop the thread

	private:

		static pthread_t		_thread;																//	Thread used for display rendering
		static bool				_terminate;																//	Flag the thread to finish
		static bool				_update;																//	Flag for a redraw in the next iteration
		static bool				_logo;																	//	Flag for printing the logo

		static const int		UPDATE_INTERVAL;														//	Interval in miliseconds for the thread main loop

		static void	* main(void * args);																//	Main loop function for the thread

};

#pragma endregion
