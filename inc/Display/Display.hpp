/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/26 11:59:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/26 12:12:49 by vzurera-         ###   ########.fr       */
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

void printOutput();
void enableRawMode();
void disableRawMode();
