/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:23 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/27 19:38:12 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Settings.hpp"

class Log {

	public:

		static void log_access(const std::string & str, const VServer * VServ = NULL);
		static void log_error(const std::string & str, const VServer * VServ = NULL);
};
