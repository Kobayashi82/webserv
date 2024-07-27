/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 14:37:32 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/27 15:38:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Settings.hpp"

struct termios orig_termios;
std::string pollon = RD "OFF";

static void signalHandler(int signum) {
	disableRawMode();
    Settings::terminate = signum + 128;
}

static void userInput() {
	char c; if (read(STDIN_FILENO, &c, 1) != 1) return ;

	if (c == 'e') { disableRawMode(); Settings::terminate = 0; }
	if (c == 'p') { if (pollon == RD "OFF") pollon = G "ON"; else pollon = RD "OFF"; }
}

static void setPadding(std::string str, std::string Color, char c, int cols, std::ostringstream & oss) {
	int Padding = ((cols + 4) - str.length()) / 2;
	if (Padding < 0) Padding = 0;
	Padding += str.length();
    oss << Color << std::setfill(c) << std::setw(Padding) << str;
}

static void setLine(std::string Color, char c, int cols, std::ostringstream & oss) {
	oss << Color << std::setw(cols + 4) << std::setfill(c) << NC << std::endl;
}

void printOutput() {
    std::ostringstream oss; winsize w; ioctl(0, TIOCGWINSZ, &w); int cols = w.ws_col - 4, row = 0;

    oss << CS CUU;
	setLine(C, '=', cols + 4, oss); row++;
	oss << C "||"; setPadding("WEBSERV", G, ' ', cols - 4, oss); oss << C "||" << std::endl; row++;
	oss << C "||"; setLine(C, '=', cols, oss); row++;
	oss << C "||"; setPadding("STATUS: " RD + pollon, Y, ' ', cols / 4, oss); oss << C "||" << std::endl; row++;

	while (++row < w.ws_row - 1)
		oss << C "||" << std::endl;

	setLine(C, '=', cols + 4, oss); row++;

	std::cout << oss.str();
	userInput(); usleep(10000);
}

void enableRawMode() {
	std::signal(SIGINT, signalHandler);
	std::cout << CHIDE CS;
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
	std::cout << CSHOW CDD CLL;
}
