/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 11:59:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/29 21:06:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Colors.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <cstdlib>
#include <csignal>

void Output();
void enableRawMode();
void disableRawMode();
