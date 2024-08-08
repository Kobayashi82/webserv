/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 18:57:42 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 16:01:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sstream>																						//	For std::ostringstream to format strings
#include <iomanip>																						//	For stream manipulators like std::setw and std::setfill

#include <ctime>																						//	For time-related functions and types

class Timer {

	private:

		//	Variables
    	std::time_t start_time;																			//	Initial time for the calculation

	public:

		//	Constructors
    	Timer() { start_time = std::time(0); }															//	Default constructor
		Timer(const Timer & src) { *this = src; }														//	Copy constructor

		//	Overloads
		Timer &		operator=(const Timer & rhs);														//	Overload for asignation
		bool		operator==(const Timer & rhs) const;												//	Overload for comparison

		//	Methods
    	double		elapsed_seconds() const;															//	Get the elapsed time in seconds from the start of the program
    	std::string	elapsed_time() const;																//	Get the elapsed time in HH:MM:SS format
		std::string	current_time() const;																//	Get the current time in HH:MM:SS format
		std::string	current_date() const;																//	Get the current date in DD:MM:YYYY format

};
