/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Monitor.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/03 13:52:15 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/09 22:18:43 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Utils.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <fstream>																						//	For file stream classes like std::ifstream, std::ofstream
#include <sstream>																						//	For std::stringstream to format strings
#include <iomanip>																						//	For stream manipulators like std::setw and std::setfill

#include <ctime>																						//	For time-related functions and types
#include <cmath>																						//	For standard math functions

#include <unistd.h>																						//	For sysconf() to calculate the page size

class Monitor {

	private:

		//	Variables (CPU)
		std::time_t		_prev_time;
		std::clock_t	_prev_cpu_time;
		std::string		_CPUinStr;
		double			_CPU;
		std::string		_MEMinStr;
		size_t			_MEM;

		//	Variables (DATA)
		size_t			_bytes_sent;
		size_t			_bytes_received;

	public:

		//	Constructor
    	Monitor();
		Monitor(const Monitor & src);																	//	Copy constructor
		~Monitor() {}

		//	Overloads
		Monitor &	operator=(const Monitor & rhs);														//	Overload for asignation

		//	CPU
		std::string	getCPUinStr();																		//	Get the CPU usage as a string
		double		getCPU();																			//	Get the CPU usage as a number

		//	MEM
		std::string	getMEMinStr();																		//	Get the MEM usage as a string
		size_t		getMEM();																			//	Get the MEM usage as a number

		//	Bytes
		void		inc_bytes_sent(size_t bytes);														//
		void		inc_bytes_received(size_t bytes);													//

		size_t		get_bytes_sent();																	//
		size_t		get_bytes_received();																//

		std::string	get_bytes_sent_str();																//
		std::string	get_bytes_received_str();															//

};

#pragma region Information

//	/proc/self/statm	Is a file that is part of the procfs filesystem, which is a virtual filesystem that provides information about the system and processes in real-time.
//	Read operations in procfs are typically very fast because they do not involve disk access, but rather read directly from the kernel memory.
//
//  size:		Total program size (pages)	- Includes both RAM and SWAP
//  resident:	Resident size (pages)		- Memory that is held in RAM
//  share:		Shared pages (pages)		- Memory that may be shared with other processes
//  text:		Text (code) (pages)			- The size of the executable code
//  lib:		Library (pages)				- The size of the loaded libraries
//  data:		Data + stack (pages)		- The size of the variables in the stack and in the heap
//  dt:			Dirty pages (pages)			- The size of the memory that have been modified but have not been synchronized with the SWAP memory

//	sysconf(_SC_PAGESIZE) obtains the system's page size at runtime and multiplies it by resident to calculate the total size in bytes.

#pragma endregion
