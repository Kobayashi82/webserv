/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:23 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/21 21:47:42 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <deque>																						//	For std::deque container
#include <queue>																						//	For std::queue container
#include <map>																							//	For std::map container

class VServer;
class Log {

	public:

		static std::ofstream *	get_fileStream(const std::string & path);
		
		enum e_type { MEM_ACCESS, MEM_ERROR, LOCAL_ACCESS, LOCAL_ERROR, BOTH_ACCESS, BOTH_ERROR };
		
		struct LogInfo {

			//	Variables
			std::string	msg;
			int			type;
			VServer *	VServ;

			//	Constructors
			LogInfo(std::string & _msg, int _type, VServer * _VServ);									//	Parameterized constructor

			//	Overloads
			LogInfo &		operator=(const LogInfo & rhs);												//	Overload for asignation
			bool			operator==(const LogInfo & rhs);											//	Overload for comparison

		};

		//	Variables
		std::deque <std::string>	access;																//	Logs for 'access' in a deque
		std::deque <std::string>	error;																//	Logs for 'error' in a deque
		std::deque <std::string>	both;																//	Logs for 'both' in a deque


		//	Constructors
    	Log();																							//	Default constructor
		Log(const Log & src);																			//	Copy constructor
		~Log();																							//	Destructor

		//	Overloads
		Log & operator=(const Log & rhs);																//	Overload for asignation

		//	Memory Log																					//	This logs are kept in memory and lost once the server closes
		void access_add(const std::string & str);
		void error_add(const std::string & str);
		void both_add(const std::string & str);
		void clear();																					//	clear all logs in 'access', 'error' and 'both'

		//	Local Log																					//	This logs are saved to a file (and also added to memory logs)
		static void	check_logs();																		//	Delete logs based on log_days config (default to 30 days)

		static void	log_access(std::string msg, int type, VServer * VServ = NULL);						//	Log to access
		static void	log_error(std::string str, int type, VServer * VServ = NULL);						//	Log to error

		static void	log(std::string msg, int type, VServer * VServ = NULL);								//	Add a new message to logs queue
		//static std::queue <LogInfo> copy_logs();														//	Copy the queue logs to process in the secundary thread
		static void process_logs();																		//	

		static void	close_fileStreams();

	private:

		size_t											_maxSize;										//	Maximum number of logs for each memory log
		static size_t									_log_days;										//	Maximum number of days to keep in local logs

		static std::queue <LogInfo>						_logs;
		static std::map <std::string, std::ofstream *>	_fileStreams;

		
		static void				log_to_file2(const std::string & msg, std::string path);

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
//			access:     Requests, responses, and other information that are not errors.
//			error:      All errors, including server-specific errors.
//			both:       Both Access and Error logs.
//
//		In addition to these 3 main logs, each virtual server has the same 3 types of logs.
//		This allows control over what happens in each virtual server.
//
//	LOCAL LOGS
//
//		Local logs are stored in files on the disk, and their location is obtained from the configuration file.
//		It is necessary to specify the location of the log files, or they will not be saved.

#pragma endregion
