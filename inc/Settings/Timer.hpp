/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 18:57:42 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/29 21:10:39 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctime>
#include <iomanip>
#include <sstream>

class Timer {

	private:

		//	Variables
    	std::time_t start_time;

	public:

		//	Constructors
    	Timer() { start_time = std::time(0); }
		Timer(const Timer & src) { *this = src; }

		//	Overloads
		Timer &		operator=(const Timer & rhs) { if (this != &rhs) { start_time = rhs.start_time; } return (*this); }
		bool		operator==(const Timer & rhs) const { return (start_time == rhs.start_time); }

		//	Methods
    	double		elapsed_seconds() const;
    	std::string	elapsed_time() const;
		std::string	current_time() const;
		std::string	current_date() const;

};
