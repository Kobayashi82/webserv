/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mutex.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:01:21 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/21 11:41:13 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Settings.hpp"
#include "Net.hpp"

#include <iostream>																						//	For strings and standard input/output like std::cin, std::cout
#include <pthread.h>

class Mutex {

	private:

		static pthread_t		thrd_disp;
		static pthread_mutex_t	mutex;

	public:

		enum 	e_action { THRD_CREATE, THRD_JOIN, THRD_DETACH, MTX_INIT, MTX_LOCK, MTX_UNLOCK, MTX_DESTROY };

		static int			thrd_set(int action);

		static int			mtx_set		(int action);
		static void 		set_string	(std::string & value1, const std::string & value2);
		static std::string	get_string	(const std::string & value1);
		static void			set_int		(int & value1, int & value2);
		static int			get_int		(int & value1);
		static void			set_long	(long & value1, long & value2);
		static long			get_long	(long & value1);
};
