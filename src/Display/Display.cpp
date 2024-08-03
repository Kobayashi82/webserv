/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 14:37:32 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/04 00:45:10 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Settings.hpp"
#include "Monitor.hpp"

#pragma region Variables

	int						Display::cols = 0;
	int						Display::rows = 0;
	int						Display::log_rows = 0;
	bool					Display::drawing = false;
	bool					Display::redraw = false;

	static struct termios	orig_termios;
	static bool				signalRegistered = false;
	static					Monitor monitor;

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
			if (cols < 0) return ;
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

#pragma region Signals

	static void killHandler(int signum) {
		Display::disableRawMode();
		Settings::terminate = signum + 128;
	}

	static void resizeHandler(int signum) {
		(void) signum;
		Display::Output();
	}

	static void stopHandler(int signum) {
		(void) signum;
		Display::disableRawMode();
		raise(SIGSTOP);
	}

    static void resumeHandler(int signum) {
        (void) signum;
        Display::enableRawMode();
        Display::Output();
    }

	static void quitHandler(int signum) {
     	Display::disableRawMode();
		Settings::terminate = signum + 128;
 	}

#pragma endregion

#pragma region Raw Mode

	#pragma region Enable

		void Display::enableRawMode() {
			if (!signalRegistered) {
				std::signal(SIGINT, killHandler);
				std::signal(SIGWINCH, resizeHandler);
				std::signal(SIGTSTP, stopHandler);
				std::signal(SIGCONT, resumeHandler);
				std::signal(SIGQUIT, quitHandler);
				signalRegistered = true;
			}
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

#pragma region Display

	#pragma region Input

		void Display::Input() {
			char c, seq[2];
			if (read(STDIN_FILENO, &c, 1) != 1) return ;												//	This is Non-Blocking
			if (c == 'e' || c == 'x') { disableRawMode(); Settings::terminate = 0; }
			if (Settings::vserver.size() > 1 && c == '\033') {											//	Escape character
                if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
                    if (seq[0] == '[') {
                        if (seq[1] == 'D') {															//	Right arrow
							if (Settings::current_vserver == Settings::vserver.size() - 1)
								Settings::current_vserver = 0;
							else
								Settings::current_vserver++;
							Output();
                        } else if (seq[1] == 'C') {														//	Left arrow
							if (Settings::current_vserver == 0)
								Settings::current_vserver = Settings::vserver.size() - 1;
							else
								Settings::current_vserver--;
							Output();
                        }
                    }
                }
			}
			if (c == 'w' && Settings::vserver.size() > 0) {												//	(S)tart / (S)top
				if (Settings::status)
					Log::log_access("WebServ 1.0 stoped");
				else
					Log::log_access("WebServ 1.0 started");
				Settings::status = !Settings::status; Output();
			}
			if (c == 'v' && Settings::status && Settings::vserver.size() > 0) {							//	(V)server start
				if (Settings::vserver[Settings::current_vserver].status)
					Log::log_access("VServer stoped");
				else
					Log::log_access("VServer started");
				Settings::vserver[Settings::current_vserver].status = !Settings::vserver[Settings::current_vserver].status; Output();
			}
		}

	#pragma endregion

	#pragma region Output

		//	┌───┐ ◄ ►
		//	├─┬─┤  ▲
		//	├─┴─┤  ▼
		//	├─┬─┤  █
		//	├─┼─┤
		//	├─┴─┤  ▒
		//	└───┘

		void Display::Output() {
			if (Display::drawing) return;
			Display::drawing = true;
			std::ostringstream oss; winsize w; ioctl(0, TIOCGWINSZ, &w); int cols = w.ws_col - 4, row = 0;
			Display::cols = cols; Display::rows = w.ws_row; Display::log_rows = Display::rows - 5;
			std::string LArrow = "◄ "; std::string RArrow = "► ";
			std::string Status;

			if (Settings::vserver.size() < 2) { LArrow = "  "; RArrow = "  "; }
			if (Settings::status) Status = G; else Status = RD;

			std::ostringstream ss; ss << Settings::vserver.size();
			std::string temp = ss.str();
			
			oss << CHIDE CS CUU;
			oss << C "┌─────────────────┬"; setLine(C, "─", (cols + 2) - 18, oss); oss << "┐" NC << std::endl; row++;
			oss << C "│ V-Servers: " G << temp; setLine(C, " ", 5 - temp.size(), oss); oss << C "│"; setPadding("WEBSERV 1.0", Status, " ", (cols + 2) - 20, 1, oss); oss << RD "X " C "│" NC << std::endl; row++;
			oss << C "├─────────────────┤"; setLine(Status, "▄", (cols + 2) - 18, oss); oss << C "│" NC << std::endl; row++;

			if (Settings::vserver.size() > 0 && Settings::vserver[Settings::current_vserver].status) Status = G; else Status = RD;
			if (Settings::status == false) Status = RD;

			temp = monitor.get_memory_str();
			oss << C "│ MEM: " G << temp; setLine(C, " ", 11 - temp.size(), oss);  oss << C "│"; setLine(Status, "▀", (cols + 2) - 18, oss); oss << C "│" NC << std::endl; row++;
			
			temp = monitor.getCPUinStr();
			oss << C "│ CPU: " G << temp; setLine(C, " ", 11 - temp.size(), oss); oss << C "│ " Y << LArrow;

			ss.str("");
			if (Settings::vserver.size() > 0) {
				if ((Settings::vserver[Settings::current_vserver].get("server_name").empty() || Settings::vserver[Settings::current_vserver].get("server_name") == "_") && Settings::current_vserver == 0) temp = "(0) Default";
				else if (Settings::vserver[Settings::current_vserver].get("server_name").empty()) {
					ss << Settings::current_vserver;
					temp = "(" + ss.str() + ") V-Server";
				} else {
					ss << Settings::current_vserver;
					temp = "(" + ss.str() + ") " + Settings::vserver[Settings::current_vserver].get("server_name");
				}
			} else
				temp = "No virtual servers available";

			setPadding(temp, Status, " ", (cols + 2) - 27, 1, oss);
			oss << "    " Y << RArrow << C "│" NC << std::endl; row++;

			oss << C "├─────────────────┴"; setLine(C, "─", (cols + 2) - 18, oss); oss << "┤" NC << std::endl; row++;

			size_t i = 0;
			if (Log::Both.size() > static_cast<size_t>(Display::log_rows)) i = Log::Both.size() - Display::log_rows;
			while (++row < Display::rows - 3) {
				std::string temp = ""; std::string isRD = "";
				if (i < Log::Both.size()) temp = Log::Both[i++];
				if (temp.empty()) { oss << C "│"; setLine(C, " ", cols + 2, oss); oss << "│" NC << std::endl; continue; }
				if (temp.find(RD) == 0) isRD = RD;
				if (temp.size() - isRD.size() > static_cast<size_t>(cols + 2)) temp = temp.substr(0, isRD.size() + cols - 1) + "...";
				int length = (cols + 2) - static_cast<int>(temp.size());
				if (length < 0) length = 0;
				oss << C "│" NC << temp;
				setLine(C, " ", length, oss);
				oss << "│" NC << std::endl;
			}

			int	length = (cols + 2) - 9;
			if (Settings::vserver.size() > 0) {
				oss << C "├────────┬───────────┬───────────┬"; setLine(C, "─", (cols + 2) - 33, oss); oss << "┤" NC << std::endl; row++;
				oss << C "│ " Y "(" G "E" Y ")xit " C "│" << Y " (" G "W" Y ")ebServ " C "│ " Y "(" G "V" Y ")server " C "│";
				length = (cols + 2) - 33;
			} else {
				oss << C "├────────┬"; setLine(C, "─", (cols + 2) - 9, oss); oss << "┤" NC << std::endl; row++;
				oss << C "│ " Y "(" G "E" Y ")xit " C "│";
			}
			if (false) {
				setLine(G, "▒", (20 * length) / 100, oss); setLine(W, "▒", (80 * length) / 100, oss); oss << C "│" NC << std::endl; row++;
			} else {
				setLine(NC, " ", length, oss); oss << C "│" NC << std::endl; row++;
			}
			if (Settings::vserver.size() > 0) {
				oss << C "└────────┴───────────┴───────────┴"; setLine(C, "─", (cols + 2) - 33, oss); oss << "┘" NC << std::endl; row++;
			} else {
				oss << C "└────────┴"; setLine(C, "─", (cols + 2) - 9, oss); oss << "┘" NC << std::endl; row++;
			}

			std::cout << oss.str();
			Display::drawing = false;
			Display::redraw = false;
		}

	#pragma endregion

#pragma endregion
