/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Thread.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:01:23 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/20 12:53:05 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Thread.hpp"

#pragma region Thread

	int		Thread::thread_set(pthread_t & thread, int action, void * (*func)(void *), void * args) {
		switch (action) {
			case THRD_CREATE: return (func == NULL || pthread_create(&thread, NULL, func, args));
			case THRD_JOIN: return (pthread_join(thread, NULL));
			case THRD_DETACH: return (pthread_detach(thread));
			default: return (0);
		}
	}

#pragma endregion

#pragma region Conditional Var

	int		Thread::cond_set(pthread_cond_t & cond, pthread_mutex_t * mutex, int action) {				//	Initializes, wait, signal, broadcast or destroys a conditional var
		switch (action) {
			case COND_INIT: return (pthread_cond_init(&cond, NULL));
			case COND_WAIT: return (pthread_cond_wait(&cond, mutex));
			case COND_SIGNAL: return (pthread_cond_signal(&cond));
			case COND_BROADCAST: return (pthread_cond_broadcast(&cond));
			case COND_DESTROY: return (pthread_cond_destroy(&cond));
			default: return (0);
		}
	}

#pragma endregion

#pragma region Mutex

	#pragma region Set

		int		Thread::mutex_set(pthread_mutex_t & mutex, int action) {									//	Initializes, locks, unlocks, or destroys a mutex
			switch (action) {
				case MTX_INIT: return (pthread_mutex_init(&mutex, NULL));
				case MTX_LOCK: return (pthread_mutex_lock(&mutex));
				case MTX_UNLOCK: return (pthread_mutex_unlock(&mutex));
				case MTX_DESTROY: return (pthread_mutex_destroy(&mutex));
				default: return (0);
			}
		}

	#pragma endregion

	#pragma region String

		std::string Thread::get_string(pthread_mutex_t & mutex, const std::string & value1) {				// Retrieves the value of a string protected by a mutex
			std::string result;

			mutex_set(mutex, MTX_LOCK);
				result = value1;
			mutex_set(mutex, MTX_UNLOCK);
			return (result);
		}

		void Thread::set_string(pthread_mutex_t & mutex, std::string & value1, const std::string value2) {	// Sets the value of a string protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1 = value2;
			mutex_set(mutex, MTX_UNLOCK);
		}

	#pragma endregion

	#pragma region Bool

		bool		Thread::get_bool(pthread_mutex_t & mutex, bool & value1) {								//	Retrieves the value of an integer protected by a mutex
			bool result;

			mutex_set(mutex, MTX_LOCK);
				result = value1;
			mutex_set(mutex, MTX_UNLOCK);
			return (result);
		}

		void	Thread::set_bool(pthread_mutex_t & mutex, bool & value1, bool value2) {						//	Sets the value of an integer protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1 = value2;
			mutex_set(mutex, MTX_UNLOCK);
		}

	#pragma endregion

	#pragma region Size_t

		size_t	Thread::get_size_t(pthread_mutex_t & mutex, size_t & value1) {								//	Retrieves the value of an size_t protected by a mutex
			size_t result;

			mutex_set(mutex, MTX_LOCK);
				result = value1;
			mutex_set(mutex, MTX_UNLOCK);
			return (result);
		}

		void	Thread::set_size_t(pthread_mutex_t & mutex, size_t & value1, size_t value2) {				//	Sets the value of an size_t protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1 = value2;
			mutex_set(mutex, MTX_UNLOCK);
		}

		void	Thread::inc_size_t(pthread_mutex_t & mutex, size_t & value1) {								//	Increases the value of an size_t protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1++;
			mutex_set(mutex, MTX_UNLOCK);
		}

		void	Thread::inc_size_t(pthread_mutex_t & mutex, size_t & value1, size_t value2) {				//	Increases the value of an size_t protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1 += value2;
			mutex_set(mutex, MTX_UNLOCK);
		}

		void	Thread::dec_size_t(pthread_mutex_t & mutex, size_t & value1) {								//	Decreases the value of an size_t protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1--;
			mutex_set(mutex, MTX_UNLOCK);
		}

		void	Thread::dec_size_t(pthread_mutex_t & mutex, size_t & value1, size_t value2) {				//	Decreases the value of an size_t protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1 -= value2;
			mutex_set(mutex, MTX_UNLOCK);
		}

	#pragma endregion

	#pragma region Integer

		int		Thread::get_int(pthread_mutex_t & mutex, int & value1) {									//	Retrieves the value of an integer protected by a mutex
			int result;

			mutex_set(mutex, MTX_LOCK);
				result = value1;
			mutex_set(mutex, MTX_UNLOCK);
			return (result);
		}

		void	Thread::set_int(pthread_mutex_t & mutex, int & value1, int value2) {						//	Sets the value of an integer protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1 = value2;
			mutex_set(mutex, MTX_UNLOCK);
		}

		void	Thread::inc_int(pthread_mutex_t & mutex, int & value1) {									//	Increases the value of an integer protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1++;
			mutex_set(mutex, MTX_UNLOCK);
		}

		void	Thread::dec_int(pthread_mutex_t & mutex, int & value1) {									//	Decreases the value of an integer protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1--;
			mutex_set(mutex, MTX_UNLOCK);
		}

	#pragma endregion

	#pragma region Long

		long	Thread::get_long(pthread_mutex_t & mutex, long & value1) {									//	Retrieves the value of a long integer protected by a mutex
			long result;

			mutex_set(mutex, MTX_LOCK);
				result = value1;
			mutex_set(mutex, MTX_UNLOCK);
			return (result);
		}

		void	Thread::set_long(pthread_mutex_t & mutex, long & value1, long value2) {						//	Sets the value of a long integer protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1 = value2;
			mutex_set(mutex, MTX_UNLOCK);
		}

		void	Thread::inc_long(pthread_mutex_t & mutex, long & value1) {									//	Increases the value of an long integer protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1++;
			mutex_set(mutex, MTX_UNLOCK);
		}

		void	Thread::dec_long(pthread_mutex_t & mutex, long & value1) {									//	Decreases the value of an long integer protected by a mutex
			mutex_set(mutex, MTX_LOCK);
				value1--;
			mutex_set(mutex, MTX_UNLOCK);
		}

	#pragma endregion

#pragma endregion
