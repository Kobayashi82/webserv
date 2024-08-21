/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mutex.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:01:23 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/21 12:06:17 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Mutex.hpp"

#pragma region Variables

	pthread_t		Mutex::thrd_disp;
	pthread_mutex_t	Mutex::mutex;

#pragma endregion

	int		Mutex::thrd_set(int action) {													//	Initializes, locks, unlocks, or destroys a mutex
		switch (action) {
			case THRD_CREATE: return (pthread_create(&thrd_disp, NULL, Display::main_display, NULL));
			case THRD_JOIN: return (pthread_join(thrd_disp, NULL));
			case THRD_DETACH: return (pthread_detach(thrd_disp));
			default: return (0);
		}
	}





#pragma region Set

	int		Mutex::mtx_set(int action) {													//	Initializes, locks, unlocks, or destroys a mutex
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

	void Mutex::set_string(std::string & value1, const std::string & value2) {			// Sets the value of a string protected by a mutex
		mtx_set(MTX_LOCK);
		value1 = value2;
		mtx_set(MTX_UNLOCK);
	}

	std::string Mutex::get_string(const std::string & value1) {		              	   // Retrieves the value of a string protected by a mutex
		std::string result;

		mtx_set(MTX_LOCK);
		result = value1;
		mtx_set(MTX_UNLOCK);
		return (result);
	}

#pragma endregion

#pragma region Integer

	void	Mutex::set_int(int & value1, int & value2) {									//	Sets the value of an integer protected by a mutex
		mtx_set(MTX_LOCK);
		value1 = value2;
		mtx_set(MTX_UNLOCK);
	}

	int		Mutex::get_int(int & value1) {												//	Retrieves the value of an integer protected by a mutex
		int	result;

		mtx_set(MTX_LOCK);
		result = value1;
		mtx_set(MTX_UNLOCK);
		return (result);
	}

#pragma endregion

#pragma region Long

	void	Mutex::set_long(long & value1, long & value2) {								//	Sets the value of a long integer protected by a mutex
		mtx_set(MTX_LOCK);
		value1 = value2;
		mtx_set(MTX_UNLOCK);
	}

	long	Mutex::get_long(long & value1) {												//	Retrieves the value of a long integer protected by a mutex
		long	result;

		mtx_set(MTX_LOCK);
		result = value1;
		mtx_set(MTX_UNLOCK);
		return (result);
	}

#pragma endregion
