/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Colors.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 19:59:03 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/27 15:28:28 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define NC   	"\033[0m"		 // Default
#define RD		"\033[1;31m"	 // Bold Red
#define G     	"\033[1;32m"	 // Bold Gree
#define Y     	"\033[1;33m"	 // Bold Yellow
#define B     	"\033[1;34m"	 // Bold Blue
#define M     	"\033[1;35m"	 // Bold Magenta
#define C     	"\033[1;36m"	 // Bold Cyan
#define W     	"\033[1;37m"	 // Bold White
#define BR 		"\033[38;5;130m" // Light Brown

#define UN 		"\033[4m"		 // Underline
#define IT 		"\033[3m"		 // Italic

#define CS		"\033[2J"		 // Clear Screen

#define CU		"\033[A"		 // Move the cursor up
#define CD		"\033[B"		 // Move the cursor down
#define CL		"\033[D"		 // Move the cursor one position to the left
#define CR		"\033[C"		 // Move the cursor one position to the right
#define CLL		"\033[1G" 		 // Move the cursor to the beginning of the line
#define CRR		"\033[F"         // Move the cursor to the end of the line

#define CUU		"\033[H"		 // Move the cursor to the initial position of the terminal (top of everything)
#define CDD		"\033[999;999H"  // Move the cursor to the end of the screen

#define CHIDE	"\033[?25l"		 // Hide the cursor
#define CSHOW	"\033[?25h"		 // Show the cursor
