/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:23 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 15:50:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Settings.hpp"

class Log {

	public:

		//	Variables
		static std::deque <std::string>	access;															//	Logs for access in a deque
		static std::deque <std::string>	error;															//	Logs for error in a deque
		static std::deque <std::string>	both;															//	Logs for both in a deque

		//	Memory Log																					//	This logs are kept in memory and lost once the server closes
		static std::string get_access(VServer * VServ = NULL, size_t rows = 200);						//	Return the logs in Access
		static std::string get_error(VServer * VServ = NULL, size_t rows = 200);						//	Return the logs in Error
		static std::string get_both(VServer * VServ = NULL, size_t rows = 200);							//	Return the logs in Both
		static void clear();																			//	clear all logs in Access, Error and Both

		//	Local Log																					//	This logs are saved to a file
		static void	log_access(std::string str, VServer * VServ = NULL);								//	Write to the Access log file
		static void	log_error(std::string str, VServer * VServ = NULL);									//	Write to the Error log file

};

#pragma region Information

//	There are 2 types of logs:
//
//	MEMORY LOGS
//
//		Memory logs are stored in variables and are lost when the server shuts down.
//
//		There are 3 main logs:
//
//			Access:     Requests, responses, and other information that are not errors.
//			Error:      All errors, including server-specific errors.
//			Both:       Both Access and Error logs.
//
//		In addition to these 3 main logs, each virtual server has the same 3 types of logs.
//		This allows control over what happens in each virtual server.
//
//	LOCAL LOGS
//
//		Local logs are stored in files on the disk, and their location is obtained from the configuration file.
//		It is necessary to specify the location of the log files, or they will not be saved.

#pragma endregion
