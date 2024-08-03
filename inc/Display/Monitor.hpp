/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Monitor.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/03 13:52:15 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/03 14:45:20 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctime>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unistd.h>

class Monitor {

	private:

		std::time_t		_prev_time;
		std::clock_t	_prev_cpu_time;
		std::string		_CPUinStr;
		double			_CPU;

		size_t			_bytes_sent;
		size_t			_bytes_received;

	public:

    	Monitor();

		std::string	getCPUinStr();
		double		getCPU();

		size_t		get_memory();
		std::string	get_memory_str();

		void		inc_bytes_sent(size_t bytes);
		void		inc_bytes_received(size_t bytes);

		size_t		get_bytes_sent();
		size_t		get_bytes_received();

		std::string	get_bytes_sent_str();
		std::string	get_bytes_received_str();

		std::string	valueToString(double Value);
		std::string	formatSize(size_t bytes);

};
