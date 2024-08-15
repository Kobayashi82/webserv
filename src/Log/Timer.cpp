/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Timer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:17:42 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/15 17:35:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Timer.hpp"

#pragma region Constructors

	Timer::Timer() : start_time(std::time(0)) {}
    Timer::Timer(const Timer & src) { *this = src; }
	Timer::~Timer() {}

#pragma endregion

#pragma region Overloads

	Timer &		Timer::operator=(const Timer & rhs) { if (this != &rhs) { start_time = rhs.start_time; } return (*this); }
	bool		Timer::operator==(const Timer & rhs) const { return (start_time == rhs.start_time); }

#pragma endregion

#pragma region Timer

    #pragma region Elapsed Seconds

        //	Get the elapsed time in seconds from the start of the program
        double Timer::elapsed_seconds() const { return static_cast<double>(std::time(0) - start_time); }

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