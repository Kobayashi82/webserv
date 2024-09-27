/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:17:42 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/27 14:07:53 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Timer.hpp"

#pragma region Constructors

	Timer::Timer() : start_time(std::time(0)) {}
    Timer::Timer(const Timer & src) { *this = src; }

#pragma endregion

#pragma region Overloads

	Timer &		Timer::operator=(const Timer & rhs) { if (this != &rhs) { start_time = rhs.start_time; } return (*this); }
	bool		Timer::operator==(const Timer & rhs) const { return (start_time == rhs.start_time); }

#pragma endregion

#pragma region Timer

    #pragma region Elapsed Seconds

        //	Get the elapsed time in seconds from the start of the program
        long Timer::elapsed_seconds() const { return static_cast<long>(std::time(0) - start_time); }

    #pragma endregion

    #pragma region Elapsed Milliseconds

        //	Get the elapsed time in seconds from the start of the program
        long Timer::elapsed_milliseconds(struct timeval start) const {
			struct timeval end_time; gettimeofday(&end_time, NULL);
			return ((end_time.tv_sec - start.tv_sec) * 1000 + (end_time.tv_usec - start.tv_usec) / 1000);
		}

    #pragma endregion

    #pragma region Elapsed Time

        //	Get the elapsed time in HH:MM:SS format
        std::string Timer::elapsed_time() const {
            int elapsed = static_cast<int>(std::time(0) - start_time);

            int hours = elapsed / 3600;
            int minutes = (elapsed % 3600) / 60;
            int seconds = elapsed % 60;

            std::ostringstream oss;
            oss << std::setw(2) << std::setfill('0') << hours << ":"
                << std::setw(2) << std::setfill('0') << minutes << ":"
                << std::setw(2) << std::setfill('0') << seconds;
            return (oss.str());
        }

    #pragma endregion

    #pragma region Current Time

        //	Get the current time in HH:MM:SS format
        std::string Timer::current_time() const {
            std::time_t now = std::time(0);
            std::tm * local_time = std::localtime(&now);

            int hours = local_time->tm_hour;
            int minutes = local_time->tm_min;
            int seconds = local_time->tm_sec;

            std::ostringstream oss;
            oss << std::setw(2) << std::setfill('0') << hours << ":"
                << std::setw(2) << std::setfill('0') << minutes << ":"
                << std::setw(2) << std::setfill('0') << seconds;
            return (oss.str());
        }

		std::string Timer::current_time_header() const {
			time_t now = time(0);
			struct tm* timeInfo = gmtime(&now);

			char date[30];
			strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);

			return (std::string(date));
		}

    #pragma endregion

    #pragma region Current Date

        //	Get the current date in DD:MM:YYYY format
        std::string Timer::current_date() const {
            std::time_t now = std::time(0);
            std::tm* local_time = std::localtime(&now);
            char buffer[20]; std::strftime(buffer, sizeof(buffer), "%d/%m/%Y", local_time);

            return (std::string(buffer));
        }

    #pragma endregion

#pragma endregion
