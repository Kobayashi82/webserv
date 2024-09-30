/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Monitor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/03 14:10:10 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/30 23:22:08 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Monitor.hpp"

#pragma region Constructors

	Monitor::Monitor() {
		_prev_time = std::time(0);
		_prev_cpu_time = std::clock();

		_CPUinStr = "0.08 %";
		_CPU = 0;
		
		_MEMinStr = "0.00 MB";
		_MEM = 0;
	}

    Monitor::Monitor(const Monitor & src) { *this = src; }

#pragma endregion

#pragma region Overloads

	Monitor & Monitor::operator=(const Monitor & rhs) {
        if (this != &rhs) { _prev_time = rhs._prev_time; _prev_cpu_time = rhs._prev_cpu_time; _CPU = rhs._CPU; _CPUinStr = rhs._CPUinStr; }
		return (*this);
    }

#pragma endregion

#pragma region CPU

	std::string Monitor::getCPUinStr() {
		std::ostringstream oss;
		std::time_t curr_time = std::time(0);
		std::clock_t curr_cpu_time = std::clock();

		double wall_time = difftime(curr_time, _prev_time);
		double cpu_time = static_cast<double>(curr_cpu_time - _prev_cpu_time) / CLOCKS_PER_SEC;
		_prev_time = curr_time; _prev_cpu_time = curr_cpu_time;

		if (wall_time == 0) return (_CPUinStr);

		oss << std::fixed << std::setprecision(2) << std::max(static_cast<double>(std::min((cpu_time / wall_time) * 100, static_cast<double>(100))), static_cast<double>(0)) << " %";
		_CPUinStr = oss.str();
		return (_CPUinStr);
	}

	double Monitor::getCPU() {
		std::ostringstream oss;
		std::time_t curr_time = std::time(0);
		std::clock_t curr_cpu_time = std::clock();

		double wall_time = difftime(curr_time, _prev_time);
		double cpu_time = static_cast<double>(curr_cpu_time - _prev_cpu_time) / CLOCKS_PER_SEC;
		_prev_time = curr_time; _prev_cpu_time = curr_cpu_time;

		if (wall_time == 0) return (_CPU);
		_CPU = (cpu_time / wall_time) * 100;
		return (_CPU);
	}

#pragma endregion

#pragma region MEM

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

	size_t Monitor::getMEM() {
		std::ifstream statm("/proc/self/statm");														//  Open the /proc/self/statm file for reading
		if (!statm.is_open()) return (_MEM);
		size_t size, resident, share, text, lib, data, dt;
		statm >> size >> resident >> share >> text >> lib >> data >> dt;								//  Read the values from the file into the variables
		_MEM = resident * sysconf(_SC_PAGESIZE);														//  Calculate and return the resident memory size in bytes
		return (_MEM);
	}

	std::string Monitor::getMEMinStr() {
		std::ifstream statm("/proc/self/statm");														//  Open the /proc/self/statm file for reading
		if (!statm.is_open()) return (_MEMinStr);
		size_t size, resident, share, text, lib, data, dt;
		statm >> size >> resident >> share >> text >> lib >> data >> dt;								//  Read the values from the file into the variables
		_MEMinStr = Utils::formatSize(resident * sysconf(_SC_PAGESIZE));								//  Calculate and return the resident memory size in bytes
		return (_MEMinStr);
	}

#pragma endregion
