/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Thread.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:01:21 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/20 12:55:21 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <pthread.h>																					//	For multi-threading and synchronization

#pragma region Thread

	class Thread {

		public:

			//	Enumerators
			enum e_action {
				THRD_CREATE, THRD_JOIN, THRD_DETACH, MTX_INIT, MTX_LOCK, MTX_UNLOCK, MTX_DESTROY,
				COND_INIT, COND_WAIT, COND_SIGNAL, COND_BROADCAST, COND_DESTROY
			};

			//	Methods
			static int			thread_set	(pthread_t & thread, int action, void * (*func)(void *) = NULL, void * args = NULL);

			static int			mutex_set	(pthread_mutex_t & mutex, int action);
			static int			cond_set	(pthread_cond_t & cond, pthread_mutex_t * mutex, int action);

			static void 		set_string	(pthread_mutex_t & mutex, std::string & value1, const std::string value2);
			static std::string	get_string	(pthread_mutex_t & mutex, const std::string & value1);
			
			static void			set_bool	(pthread_mutex_t & mutex, bool & value1, bool value2);
			static bool			get_bool	(pthread_mutex_t & mutex, bool & value1);
			
			static void			set_size_t	(pthread_mutex_t & mutex, size_t & value1, size_t value2);
			static void			inc_size_t	(pthread_mutex_t & mutex, size_t & value1);
			static void			inc_size_t	(pthread_mutex_t & mutex, size_t & value1, size_t value2);
			static void			dec_size_t	(pthread_mutex_t & mutex, size_t & value1);
			static void			dec_size_t	(pthread_mutex_t & mutex, size_t & value1, size_t value2);
			static size_t		get_size_t	(pthread_mutex_t & mutex, size_t & value1);

			static void			set_int		(pthread_mutex_t & mutex, int & value1, int value2);
			static void			inc_int		(pthread_mutex_t & mutex, int & value1);
			static void			dec_int		(pthread_mutex_t & mutex, int & value1);
			static int			get_int		(pthread_mutex_t & mutex, int & value1);

			static void			set_long	(pthread_mutex_t & mutex, long & value1, long value2);
			static void			inc_long	(pthread_mutex_t & mutex, long & value1);
			static void			dec_long	(pthread_mutex_t & mutex, long & value1);
			static long			get_long	(pthread_mutex_t & mutex, long & value1);
	};

#pragma endregion
