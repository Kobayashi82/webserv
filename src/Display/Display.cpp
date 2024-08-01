/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 14:37:32 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/02 01:04:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Settings.hpp"

#pragma region Variables

	int				Display::cols = 0;
	int				Display::rows = 0;
	int				Display::log_rows = 0;

	struct termios	orig_termios;
	std::string		pollon = RD "OFF";

#pragma endregion

#pragma region Utils

	#pragma region Set Padding

		static void setPadding(std::string str, std::string Color, std::string c, int cols, int div, std::ostringstream & oss) {
			int Padding = ((cols) - str.length()) / (2 * div);
			if (Padding < 0) Padding = 0;
			
			for (int i = 0; i < Padding; ++i) Color += c;
			oss << Color << str;
			Color = "";
			for (int i = Padding + str.length(); i < cols; ++i) Color += c;
			oss << Color;
		}


	#pragma endregion

	#pragma region Set Line

		static void setLine(std::string Color, std::string c, int cols, std::ostringstream & oss) {
			for (int i = 0; i < cols; ++i) Color += c;
			oss << Color;
		}

	#pragma endregion

	#pragma region Set Terminal Size

		void Display::setTerminalSize(size_t rows, size_t cols) {
			std::cout << "\033[8;" << rows << ";" << cols << "t";
		}

	#pragma endregion

#pragma endregion

#pragma region Display

	#pragma region Input

		void Display::Input() {
			char c; if (read(STDIN_FILENO, &c, 1) != 1) return ;								//	This is Non-Blocking

			if (c == 'e') { disableRawMode(); Settings::terminate = 0; }
			if (c == 'p') { if (pollon == RD "OFF") pollon = G "ON"; else pollon = RD "OFF"; Output(); }
		}

	#pragma endregion

	#pragma region Output

		void Display::Output() {
			std::ostringstream oss; winsize w; ioctl(0, TIOCGWINSZ, &w); int cols = w.ws_col - 4, row = 0;
			Display::cols = cols; Display::rows = w.ws_row; Display::log_rows = Display::rows - 5;

			oss << CS CUU;
			oss << C "┌"; setLine(C, "─", cols + 2, oss); oss << "┐" NC << std::endl; row++;
			oss << C "│"; setPadding("WEBSERV", G, " ", cols + 2, 1, oss);	oss << C "│" NC << std::endl; row++;
			oss << C "├"; setLine(C, "─", cols + 2, oss); oss << "┤" NC; row++;
			oss << C "│"; setPadding("STATUS: " RD + pollon, Y, " ", cols + 2, 4, oss); oss << C "│" NC << std::endl; row++;
			oss << C "├"; setLine(C, "─", cols + 2, oss); oss << "┤" NC; row++;
			size_t i = 0;
			if (Log::Both.size() > static_cast<size_t>(Display::log_rows)) i = Log::Both.size() - Display::log_rows;
			while (++row < w.ws_row - 1) {
				std::string temp = ""; std::string isRD = "";
				if (i < Log::Both.size()) temp = Log::Both[i++];
				if (temp.find(RD) == 0) isRD = RD;
				if (temp.size() - isRD.size() > static_cast<size_t>(cols + 2)) temp = temp.substr(0, isRD.size() + cols - 1) + "...";
				int length = (cols + 2) - (temp.size() - isRD.size());
				if (length < 0) length = 0;
				oss << C "│" NC << temp;
				setLine(C, " ", length, oss); oss << "│" NC << std::endl;
			}
			oss << C "└"; setLine(C, "─", cols + 2, oss); row++; oss << "┘" NC << std::endl;

			std::cout << oss.str();
		}

	#pragma endregion

#pragma endregion

#pragma region Raw Mode

	#pragma region Signal Handler

		static void signalHandler(int signum) {
			Display::disableRawMode();
			Settings::terminate = signum + 128;
		}

		static void resizeHandler(int signum) {
			(void) signum;
			Display::Output();
		}

	#pragma endregion

	#pragma region Enable

		void Display::enableRawMode() {
			std::signal(SIGINT, signalHandler);
			std::signal(SIGWINCH, resizeHandler);
			std::cout << CHIDE CS;
			tcgetattr(STDIN_FILENO, &orig_termios);
			struct termios raw = orig_termios;
			raw.c_lflag &= ~(ECHO | ICANON);
			tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
			int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
			fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
		}

	#pragma endregion

	#pragma region Disable

		void Display::disableRawMode() {
			tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
			std::cout << CSHOW CDD CLL;
		}

	#pragma endregion

#pragma endregion
