/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 11:59:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/23 14:19:40 by vzurera-         ###   ########.fr       */
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
#include <sys/wait.h>																					//	For waitpid. Used when a child process finish

#pragma region Display

	class Display {

		public:

			//	Variables
			static pthread_mutex_t	mutex;																//	Mutex for synchronizing access to shared resources

			static bool				drawing;															//	True if printing to the terminal
			static int				failCount;															//	Current number of failures when printing to the terminal
			static int				maxFails;															//	Maximum numbers of retries to print to the terminal if something fails
			static bool				RawModeDisabled;													//	Status of the terminal (false if in raw mode)
			static bool				ForceRawModeDisabled;												//	Force terminal in normal mode (not raw mode)
			static bool				Resized;															//	True if the terminal has been resized
			static bool				redraw;																//	Flags for a redraw of the terminal
			static bool				background;															//	True if the program is running in the background (&)
			static int				signal;																//	Last signal code

			//	Methods
			static void	signal_handler();																//	Handles signals
			static int	signal_check();																	//	Checks for signals to process
			static void	Logo();																			//	Prints the Webserv logo
			static void	Input();																		//	Manages user input
			static void	Output();																		//	Manages output to the terminal
			static void	enableRawMode();																//	Enables raw un-buffered mode for the terminal
			static void	disableRawMode();																//	Disables raw un-buffered mode for the terminal
			static void	setTerminalSize(size_t rows, size_t cols);										//	Sets the terminal size (a big buggy)

			static void	update();																		//	Requests a redraw in the next iteration
			static void	logo();																			//	Requests a logo print in the next iteration
			static int	isTerminate();																	//	Checks if the main termination flag is set
			static bool	isRawMode();																	//	Checks if the program is in raw mode

			static void	start();																		//	Starts the thread
			static void	stop();																			//	Stops the thread

		private:

			//	Variables
			static pthread_t		_thread;															//	Thread used for display rendering
			static bool				_terminate;															//	Flags the thread to finish
			static bool				_update;															//	Flags for a redraw in the next iteration
			static bool				_logo;																//	Flags for printing the logo

			static const int		UPDATE_INTERVAL;													//	Interval in milliseconds for the thread main loop

			//	Method
			static void	* main(void * args);															//	Main loop function for the thread

	};

#pragma endregion
