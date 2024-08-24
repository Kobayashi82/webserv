/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:23 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/24 13:06:24 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Timer.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <deque>																						//	For std::deque container.	Used to store memory logs
#include <queue>																						//	For std::queue container.	Used as an intermediary for thread-safe communication
#include <map>																							//	For std::map container.		Used to group logs and save them to a file in one operation

class VServer;
class Log {

	public:
	
		//	Variables
		enum e_type { MEM_ACCESS, MEM_ERROR, BOTH_ACCESS, BOTH_ERROR, LOCAL_ACCESS, LOCAL_ERROR };
		
		std::deque <std::string>	access;																//	Logs for 'access' in a deque
		std::deque <std::string>	error;																//	Logs for 'error' in a deque
		std::deque <std::string>	both;																//	Logs for 'both' in a deque

		static pthread_mutex_t		mutex;

		//	Constructors
    	Log();																							//	Default constructor
		Log(const Log & src);																			//	Copy constructor
		~Log();																							//	Destructor

		//	Overloads
		Log &	operator=(const Log & rhs);																//	Overload for asignation
		bool	operator==(const Log & rhs) const;														//	Overload for comparison

		//	Memory Log																					//	This logs are kept in memory and lost once the server closes
		void access_add(const std::string & msg);														//	Add a log to 'access'
		void error_add(const std::string & msg);														//	Add a log to 'error'
		void both_add(const std::string & msg);															//	Add a log to 'both'
		void clear();																					//	clear all logs in 'access', 'error' and 'both'

		//	Local Log																					//	This logs are saved to a file (and also added to memory logs)
		static void process_logs();																		//	Save logs to memory and/or to a file
		static void	log(std::string msg, int type, VServer * VServ = NULL, std::string path = "", long maxsize = -1);	//	Add a new message to logs queue

		static void	start();																			//	Start the thread
		static void	stop();																				//	Stop the thread
		static void	start_mutex();
		static void	release_mutex();

	private:

		struct LogInfo {

			//	Variables
			std::string	msg;
			int			type;
			VServer *	VServ;
			std::string	path;
			long		maxsize;

			//	Constructors
			LogInfo(std::string & _msg, int _type, VServer * _VServ, std::string _path, long _maxsize);	//	Parameterized constructor

			//	Overloads
			LogInfo &		operator=(const LogInfo & rhs);												//	Overload for asignation
			bool			operator==(const LogInfo & rhs);											//	Overload for comparison

		};

		static pthread_t			_thread;
		static bool					_terminate;															//	Flag the thread to finish

		static std::queue <LogInfo>	_logs;																//	Queue container with logs that need to be processed

		static const size_t			MEM_MAXSIZE;														//	Maximum number of logs for each memory log
		static long					LOCAL_MAXSIZE;														//	Maximum size of the log in disk
		static const int			UPDATE_INTERVAL;													//	Interval in miliseconds for the thread main loop

		static void	truncate_log(const std::string & path, long long maxFileSize, long long extraSize);	//	Truncate the log file to the maximum set in the config file (default to 1 MB | 0 MB = dont truncate | Max 10 MB)
		static void	log_to_memory(std::string msg, int type, VServer * VServ = NULL);					//	Log to memory
		static void	log_to_file(const std::string & msg, std::string path, long maxsize);				//	Save logs to file

		static void	* main(void * args);

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
//			both:       Both access and error logs.
//
//		In addition to these 3 main logs, each virtual server has the same 3 types of logs.
//		This allows control over what happens in each virtual server.
//
//	LOCAL LOGS
//
//		Local logs are stored in files on the disk, and their location is obtained from the configuration file.
//		It is necessary to specify the location of the log files, or they will not be saved.

#pragma endregion
