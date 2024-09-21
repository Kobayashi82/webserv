/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:23 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/21 18:46:41 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Timer.hpp"

#include <cstdlib>																						//	For std::system to run logrorate
#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <deque>																						//	For std::deque container	Used to store memory logs
#include <queue>																						//	For std::queue container	Used as an intermediary for thread-safe communication

#include <pthread.h>																					//	For multi-threading and synchronization
#include <unistd.h>																						//	For access() to checks the accessibility of logrotate
#include <sys/stat.h>																					//	For chmod to change the file permissions of the config file for logrotate

#pragma region Log

	class VServer;																						//	Forward declaration of VServer
	class Log {

		public:
		
			//	Enumarators
			enum e_type {
				MEM_ACCESS = 600, MEM_ERROR = 601,
				VSERV_ACCESS = 602, VSERV_ERROR = 603,
				GLOBAL_ACCESS = 604, GLOBAL_ERROR = 605,
				BOTH_ACCESS = 606, BOTH_ERROR = 607,
				LOCAL_ACCESS = 608, LOCAL_ERROR = 609
			};
			
			//	Variables
			std::deque <std::string>	access;															//	Logs for 'access' in a deque
			std::deque <std::string>	error;															//	Logs for 'error' in a deque
			std::deque <std::string>	both;															//	Logs for 'both' in a deque

			static pthread_mutex_t		mutex;															//	Mutex for synchronizing access to shared resources

			//	Constructors
			Log();																						//	Default constructor
			Log(const Log & src);																		//	Copy constructor
			~Log();																						//	Destructor

			//	Overloads
			Log &	operator=(const Log & rhs);															//	Overload for assignation
			bool	operator==(const Log & rhs) const;													//	Overload for comparison

			//	Memory Log
			void access_add(const std::string & msg);													//	Adds a log to 'access'
			void error_add(const std::string & msg);													//	Adds a log to 'error'
			void both_add(const std::string & msg);														//	Adds a log to 'both'
			void clear();																				//	Clears all logs in 'access', 'error' and 'both'

			//	Local Log
			static void process_logs();																	//	Saves logs to memory and/or to a file
			static std::string log_request(std::string log_msg, VServer * VServ, bool isLocal);
			static void	log(std::string msg, int type, VServer * VServ = NULL, std::vector<std::pair<std::string, std::string> > * data = NULL);	//	Adds a new message to logs queue

			//	Log Rotate
			static void exec_logrot(const std::string config_path);										//	Executes logrotate (external program to manage logs rotation)

			//	Thread
			static void	start();																		//	Starts the thread
			static void	stop();																			//	Stops the thread
			static void	release_mutex();																//	Releases the mutex

		private:

			#pragma region LogInfo

				struct LogInfo {

					//	Variables
					std::string	msg;																	//	Content of the log message
					int			type;																	//	Type of log (e_type enumarator)
					VServer *	VServ;																	//	Pointer to the associated VServer
					std::string	path;																	//	File path related to the log

					//	Constructors
					LogInfo(std::string _msg, int _type, VServer * _VServ, std::string _path);			//	Parameterized constructor

					//	Overloads
					LogInfo &		operator=(const LogInfo & rhs);										//	Overload for assignation
					bool			operator==(const LogInfo & rhs);									//	Overload for comparison

				};

			#pragma endregion

			//	Variables
			static pthread_t			_thread;														//	Thread used for processing log entries
			static bool					_terminate;														//	Flag the thread to finish

			static std::queue <LogInfo>	_logs;															//	Queue container with logs that need to be processed

			static const size_t			MEM_MAXSIZE;													//	Maximum number of logs for each memory log
			static long					LOCAL_ROTATESIZE;												//	Maximum size of the log before rotation	(default to 1 MB | 0 MB = no rotation | Max 100 MB)
			static int					LOCAL_ROTATE;													//	Number of rotations files 				(default to 7 | 0 = no rotations files | Max 100)

			//	Logs
			static void	log_to_memory(std::string msg, int type, VServer * VServ = NULL);				//	Logs to memory
			static void	log_to_file(const std::string & msg, std::string path);							//	Saves logs to file

			//	Log Rotate
			static void add_logrot(std::ofstream & oss, const std::string & log_paths, std::string size, std::string rotate, const std::string & user);	//	Adds a section to the config file for 'logrotate'
			static int create_logrot(const std::string config_path);																					//	Creates the config file for 'logrotate'
			static std::string get_logrot_path();																										//	Gets the path of 'logrotate' (if installed)

			//	Thread
			static void	* main(void * args);															//	Main loop function for the thread

	};

#pragma endregion

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

#pragma region LogRotate

	// LOGROTATE

	// logrotate -d /path/to/your/logrotate.conf				-d ejecuta logrotate en modo de depuración, mostrando lo que haría sin realizar cambios reales
	// logrotate -f /path/to/your/logrotate.conf				-f fuerza la rotación de logs, independientemente de si se cumplen las condiciones normales

	// /path/to/log {
	// 	# daily								# Rotar los logs diariamente
	// 	size 1M								# Rotar si alcanza los 50 MB (k, M, G)
	// 	rotate 7							# Mantener los últimos 7 archivos rotados
	// 	# compress							# Comprimir los logs rotados
	// 	# delaycompress							# Comprime los logs en la siguiente rotación (útil si deseas tener el archivo más reciente sin compresión)
	// 	missingok							# Evita errores si el archivo de log no existe
	// 	notifempty							# No rota el archivo si está vacío
	// 	create 0640 tu_usuario tu_usuario				# Crea un nuevo archivo de log con los permisos y propiedad especificados
	// 	# postrotate							# Acción después de rotar el archivo
	// 	#	/bin/ls
	//     endscript
	// }

#pragma endregion