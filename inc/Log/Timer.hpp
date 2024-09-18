/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 18:57:42 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/18 14:13:05 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sstream>																						//	For std::stringstream to format strings
#include <iomanip>																						//	For stream manipulators like std::setw and std::setfill
#include <ctime>																						//	For time-related functions and types

#pragma region Timer

class Timer {

	private:

		//	Variables
    	std::time_t start_time;																			//	Initial time for the calculation

	public:

		//	Constructors
    	Timer();																						//	Default constructor
		Timer(const Timer & src);																		//	Copy constructor

		//	Overloads
		Timer &		operator=(const Timer & rhs);														//	Overload for asignation
		bool		operator==(const Timer & rhs) const;												//	Overload for comparison

		//	Methods
    	double		elapsed_seconds() const;															//	Get the elapsed time in seconds from the start of the program
    	std::string	elapsed_time() const;																//	Get the elapsed time in HH:MM:SS format
		std::string	current_time() const;																//	Get the current time in HH:MM:SS format
		std::string	current_date() const;																//	Get the current date in DD:MM:YYYY format

};

#pragma endregion
