/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 18:57:42 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/27 19:23:17 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctime>
#include <iomanip>
#include <sstream>

class Timer {

	private:

    	std::time_t start_time;

	public:

    	Timer() { start_time = std::time(0); }

    	double		elapsed_seconds() const;
    	std::string	elapsed_time() const;
		std::string	current_time() const;
		std::string	current_date() const;
};
