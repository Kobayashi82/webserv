/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Monitor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/03 14:10:10 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 16:06:47 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Monitor.hpp"

Monitor::Monitor() {
	_prev_time = std::time(0);
	_prev_cpu_time = std::clock();
	_CPUinStr = "0.08 %";
	_CPU = 0;
	_bytes_sent = 0;
	_bytes_received = 0;
}

std::string Monitor::getCPUinStr() {
	std::ostringstream oss;
	std::time_t curr_time = std::time(0);
	std::clock_t curr_cpu_time = std::clock();

	double wall_time = difftime(curr_time, _prev_time);
	double cpu_time = static_cast<double>(curr_cpu_time - _prev_cpu_time) / CLOCKS_PER_SEC;
	_prev_time = curr_time; _prev_cpu_time = curr_cpu_time;

	if (wall_time == 0) return (_CPUinStr);

	oss << std::fixed << std::setprecision(2) << (cpu_time / wall_time) * 100 << " %";
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
    std::ifstream statm("/proc/self/statm");                                                            //  Open the /proc/self/statm file for reading
    size_t size, resident, share, text, lib, data, dt;
    statm >> size >> resident >> share >> text >> lib >> data >> dt;                                    //  Read the values from the file into the variables
    return (resident * sysconf(_SC_PAGESIZE));															//  Calculate and return the resident memory size in bytes
}

std::string Monitor::getMEMinStr() {
    std::ifstream statm("/proc/self/statm");                                                            //  Open the /proc/self/statm file for reading
    size_t size, resident, share, text, lib, data, dt;
    statm >> size >> resident >> share >> text >> lib >> data >> dt;                                    //  Read the values from the file into the variables
    return (formatSize(resident * sysconf(_SC_PAGESIZE)));												//  Calculate and return the resident memory size in bytes
}



void Monitor::inc_bytes_sent(size_t bytes) { _bytes_sent += bytes; }
void Monitor::inc_bytes_received(size_t bytes) { _bytes_received += bytes; }

size_t Monitor::get_bytes_sent() { return (_bytes_sent); }
size_t Monitor::get_bytes_received() { return (_bytes_received); }

std::string Monitor::get_bytes_sent_str() { return (formatSize(_bytes_sent)); }
std::string Monitor::get_bytes_received_str() { return (formatSize(_bytes_received)); }



std::string Monitor::valueToString(double Value) {																		// Convert a number to a string
    std::ostringstream oss; oss << std::fixed << std::setprecision(2) << Value; std::string Result = oss.str();         // Convert to string with sufficient precision

    size_t decimalPos = Result.find('.');
    if (decimalPos != std::string::npos) {
        size_t End = Result.find_last_not_of('0');                                                                      // Remove trailing zeros after the decimal point
        if (End != std::string::npos && End > decimalPos) Result.erase(End + 1); else Result.erase(decimalPos);         // Remove decimal point if there are only zeros after it
    } return (Result);
}

std::string Monitor::formatSize(size_t bytes) {
    const char * suffixes[] = {"byte", "KB", "MB", "GB", "TB"}; std::string s = "";
    size_t suffix = 0; double size = static_cast<double>(bytes);

    while (size >= 1024 && suffix < 4) {
        size /= 1024;
        ++suffix;
    }
    if (size > 1 && suffix == 0) s = "s";
    return (valueToString(size) + " " + suffixes[suffix] + s);
}
