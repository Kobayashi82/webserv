/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 14:37:32 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/19 22:16:22 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Settings.hpp"
#include "Net.hpp"
#include "Monitor.hpp"

#pragma region Variables

	bool					Display::drawing = false;													//	Is in the middle of an Output()
	int						Display::failCount = 0;														//	Current number of fails when printing in the the terminal
	int						Display::maxFails = 3;														//	Maximum numbers of retries to print in the terminal if something fails
	bool					Display::RawModeDisabled = true;											//	Status of the terminal (false if in raw mode)
	bool					Display::ForceRawModeDisabled = false;										//	Force terminal in normal mode (not raw mode)
	bool					Display::Resized = false;													//	True if the terminal has been resized
	bool					Display::redraw = false;													//	Is in the middle of an Output()

	static Monitor			monitor;																	//	Class to obtain MEM and CPU ussage
	static struct termios	orig_termios;																//	Structure for terminal information
	static bool				signalRegistered = false;													//	Signals already registered
	static int				total_cols = 0;																//	Number of columns in the terminal
	static int				total_rows = 0;																//	Number of rows in the terminal
	static int				log_rows = 0;																//	Number of rows available for logs or settings


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
		if (Settings::check_only || Display::RawModeDisabled || Display::ForceRawModeDisabled) std::cout << CL CL "  ";
		Settings::terminate = signum + 128;
	}

	static void resizeHandler(int signum) {
		(void) signum;
		Display::Resized = true;
	}

	static void stopHandler(int signum) {
		(void) signum;
		if (Settings::check_only || Display::RawModeDisabled || Display::ForceRawModeDisabled) std::cout << CL CL "  ";
		Display::disableRawMode();
		raise(SIGSTOP);
	}

    static void resumeHandler(int signum) {
        (void) signum;
        Display::enableRawMode();
        Display::Output();
		if (Settings::check_only || Display::RawModeDisabled || Display::ForceRawModeDisabled) Display::Logo();
    }

	static void quitHandler(int signum) {
		if (Settings::check_only || Display::RawModeDisabled || Display::ForceRawModeDisabled) std::cout << CL CL "  ";
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
			std::cout << CHIDE;
			if (Settings::check_only || ForceRawModeDisabled) return;
			if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) return;
			struct termios raw = orig_termios; raw.c_lflag &= ~(ECHO | ICANON);
			if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) { disableRawMode(); return; }
			int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
			if (flags == -1) { disableRawMode(); return; }
			if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) { disableRawMode(); return; }
			std::cout.flush();
			RawModeDisabled = false;
		}

	#pragma endregion

	#pragma region Disable

		void Display::disableRawMode() {
			if (!Settings::check_only && !RawModeDisabled && !ForceRawModeDisabled) {
				tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
				RawModeDisabled = true;
				std::cout << CDD CLL;
			}
			std::cout << CSHOW;
			std::cout.flush();
		}

	#pragma endregion

#pragma endregion

#pragma region Display

	#pragma region Input

		void Display::Input() {
			if (RawModeDisabled) return;
			char c, seq[2];
			if (read(STDIN_FILENO, &c, 1) != 1) return ;																				//	This is Non-Blocking
			if (c == '\033' && read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1 && seq[0] == '[') {
                if (Settings::vserver.size() > 0 && seq[1] == 'D') {																	//	Right arrow
					if (Settings::current_vserver == -1)
						Settings::current_vserver = static_cast<int>(Settings::vserver.size() - 1);
					else
						Settings::current_vserver--;
					if (Display::drawing) { redraw = true; return; } Output();
                } else if (Settings::vserver.size() > 0 && seq[1] == 'C') {																//	Left arrow
					if (Settings::current_vserver == static_cast<int>(Settings::vserver.size() - 1))
						Settings::current_vserver = -1;
					else
						Settings::current_vserver++;
					if (Display::drawing) { redraw = true; return; } Output();
                } else if (seq[1] == 'A') {																								//	Up arrow
					if (Settings::current_vserver == -1 && Settings::global.config_displayed == false
						&& Settings::global.log_index > 0) {
							Settings::global.log_index--; Settings::global.autolog = false;
					} else if (Settings::current_vserver == -1 && Settings::global.config_displayed == true
						&& Settings::global.config_index > 0)
							Settings::global.config_index--;
					else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == false
						&& Settings::vserver[Settings::current_vserver].log_index > 0) {
							Settings::vserver[Settings::current_vserver].log_index--;
							Settings::vserver[Settings::current_vserver].autolog = false;
					} else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == true
						&& Settings::vserver[Settings::current_vserver].config_index > 0)
							Settings::vserver[Settings::current_vserver].config_index--;
					else return ;
					if (Display::drawing) { redraw = true; return; } Output();
                } else if (seq[1] == 'B') {																								//	Down arrow
					if (Settings::current_vserver == -1 && Settings::global.config_displayed == false
						&& static_cast<int>(Settings::global.log.both.size()) >= log_rows
						&& static_cast<int>(Settings::global.log_index) < static_cast<int>(Settings::global.log.both.size()) - (log_rows - 1)) {
							Settings::global.log_index++;
							if (Settings::global.log_index == Settings::global.log.both.size() - (log_rows - 1)) Settings::global.autolog = true;
					} else if (Settings::current_vserver == -1 && Settings::global.config_displayed == true
						&& static_cast<int>(Settings::global.config.size()) >= log_rows
						&& static_cast<int>(Settings::global.config_index) < static_cast<int>(Settings::global.config.size()) - (log_rows - 1))
							Settings::global.config_index++;

					else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == false
						&& static_cast<int>(Settings::vserver[Settings::current_vserver].log.both.size()) >= log_rows
						&& static_cast<int>(Settings::vserver[Settings::current_vserver].log_index) < static_cast<int>(Settings::vserver[Settings::current_vserver].log.both.size()) - (log_rows - 1)) {
							Settings::vserver[Settings::current_vserver].log_index++;
							if (Settings::vserver[Settings::current_vserver].log_index == Settings::vserver[Settings::current_vserver].log.both.size() - (log_rows - 1)) Settings::vserver[Settings::current_vserver].autolog = true;
					} else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == true
						&& static_cast<int>(Settings::vserver[Settings::current_vserver].config.size()) >= log_rows
						&& static_cast<int>(Settings::vserver[Settings::current_vserver].config_index) < static_cast<int>(Settings::vserver[Settings::current_vserver].config.size()) - (log_rows - 1))
							Settings::vserver[Settings::current_vserver].config_index++;
					else return ;
					if (Display::drawing) { redraw = true; return; } Output();
                } else if (seq[1] == 'H') {																								//	Home
					if (Settings::current_vserver == -1 && Settings::global.config_displayed == false && Settings::global.log_index > 0) {
						Settings::global.log_index = 0;
						Settings::global.autolog = false;
					} else if (Settings::current_vserver == -1 && Settings::global.config_displayed == true && Settings::global.config_index > 0)
						Settings::global.config_index = 0;
					else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == false
						&& Settings::vserver[Settings::current_vserver].log_index > 0) {
							Settings::vserver[Settings::current_vserver].log_index = 0;
							Settings::vserver[Settings::current_vserver].autolog = false;
					} else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == true
						&& Settings::vserver[Settings::current_vserver].config_index > 0)
							Settings::vserver[Settings::current_vserver].config_index = 0;
					else return ;
					if (Display::drawing) { redraw = true; return; } Output();
            	} else if (seq[1] == 'F') {																								//	End
					size_t temp = 0;
					if (Settings::current_vserver == -1 && Settings::global.config_displayed == false && Settings::global.log.both.size() > 0
						&& Settings::global.log_index != Settings::global.log.both.size() - (log_rows - 1)) {
							if (static_cast<int>(Settings::global.log.both.size()) - (log_rows - 1) < 0) temp = 0;
							else temp = static_cast<int>(Settings::global.log.both.size()) - (log_rows - 1);
							if (Settings::global.log_index == temp) return ; else Settings::global.log_index = temp;
							if (static_cast<int>(Settings::global.log_index) == static_cast<int>(Settings::global.log.both.size()) - (log_rows - 1)) Settings::global.autolog = true;
					} else if (Settings::current_vserver == -1 && Settings::global.config_displayed == true && Settings::global.config.size() > 0) {
						if (static_cast<int>(Settings::global.config.size()) - (log_rows - 1) < 0) temp = 0;
						else temp = static_cast<int>(Settings::global.config.size()) - (log_rows - 1);
						if (Settings::global.config_index == temp) return ; else Settings::global.config_index = temp;
					} else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == false
						&& Settings::vserver[Settings::current_vserver].log.both.size() > 0) {
							if (static_cast<int>(Settings::vserver[Settings::current_vserver].log.both.size()) - (log_rows - 1) < 0) temp = 0;
							else temp = static_cast<int>(Settings::vserver[Settings::current_vserver].log.both.size()) - (log_rows - 1);
							if (Settings::vserver[Settings::current_vserver].log_index == temp) return ; else Settings::vserver[Settings::current_vserver].log_index = temp;
							if (static_cast<int>(Settings::vserver[Settings::current_vserver].log_index) == static_cast<int>(Settings::vserver[Settings::current_vserver].log.both.size()) - (log_rows - 1)) Settings::vserver[Settings::current_vserver].autolog = true;
					} else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == true
						&& Settings::vserver[Settings::current_vserver].config.size() > 0) {
							if (static_cast<int>(Settings::vserver[Settings::current_vserver].config.size()) - (log_rows - 1) < 0) temp = 0;
							else temp = static_cast<int>(Settings::vserver[Settings::current_vserver].config.size()) - (log_rows - 1);
							if (Settings::vserver[Settings::current_vserver].config_index == temp) return ; else Settings::vserver[Settings::current_vserver].config_index = temp;
					} else return ;
					if (Display::drawing) { redraw = true; return; } Output();
				}
            }
			if ((c == 'w' || c == 'W') && Settings::vserver.size() > 0) {																				//	(S)tart / (S)top
				Settings::global.status = !Settings::global.status;
				if (Settings::global.status) {
					Log::log_access("WebServ 1.0 started");
					Net::socket_create_all();
				} else {
					Log::log_access("WebServ 1.0 stoped");
					Net::socket_close_all();
				}
			} else if ((c == 'v' || c == 'V') && Settings::vserver.size() > 0
				&& Settings::current_vserver != -1) {																									//	(V)server start
					Settings::vserver[Settings::current_vserver].force_off = !Settings::vserver[Settings::current_vserver].force_off;
					if (Settings::vserver[Settings::current_vserver].status)
						Net::socket_close(&Settings::vserver[Settings::current_vserver]);
					else if (!Settings::vserver[Settings::current_vserver].force_off)
						Net::socket_create(&Settings::vserver[Settings::current_vserver]);
			} else if ((c == 'c' || c == 'C')) {																										//	(C)lear log
				if (Settings::current_vserver == -1 && Settings::global.log.both.size() > 0)
					Settings::global.log.clear();
				else if (Settings::current_vserver != -1 && Settings::vserver.size() > 0
					&& Settings::vserver[Settings::current_vserver].log.both.size() > 0)
						Settings::vserver[Settings::current_vserver].clear_logs();
				else return ;
				Output();
			} else if ((c == 'l' || c == 'L')) {																										//	(L)og
				if (Settings::current_vserver == -1 && Settings::global.config_displayed == true)
					Settings::global.config_displayed = false;
				else if (Settings::current_vserver != -1 && Settings::vserver.size() > 0
					&& Settings::vserver[Settings::current_vserver].config_displayed == true)
						Settings::vserver[Settings::current_vserver].config_displayed = false;
				else return ;
				Output();
			} else if ((c == 's' || c == 'S')) {																										//	(S)ettings
				if (Settings::current_vserver == -1 && Settings::global.config_displayed == false)
					Settings::global.config_displayed = true;
				else if (Settings::current_vserver != -1 && Settings::vserver.size() > 0
					&& Settings::vserver[Settings::current_vserver].config_displayed == false)
						Settings::vserver[Settings::current_vserver].config_displayed = true;
				else return ;
				Output();
			} else if ((c == 'e' || c == 'E')) { Settings::terminate = 0;																				//	(E)xit
			} else if ((c == 'r' || c == 'R')) { std::cout << CB CHIDE CS CUU; std::cout.clear();  drawing = false; failCount = 0; Output(); }			//	(R)eset terminal
		}

	#pragma endregion

	#pragma region Output

		#pragma region Print Log

			void print_log(const std::deque<std::string> & log, size_t index, std::ostringstream &oss, int &cols, int &row) {
				while (++row < total_rows - 3) {
					int length = 0; std::string temp = "";
					if (index < log.size()) temp = log[index++];
					if (temp.empty()) { oss << C "│"; setLine(C, " ", cols + 2, oss); oss << C "│" NC << std::endl; continue; }
					length = Utils::str_nocolor_length(temp);
					if (length > cols + 2) temp = Utils::str_nocolor_trunc(temp, cols - 1);
					length = (cols + 2) - Utils::str_nocolor_length(temp);
					if (length < 0) length = 0;
					oss << C "│" NC << temp;
					setLine(C, " ", length, oss);
					oss << C "│" NC << std::endl;
				}
			}

		#pragma endregion

		#pragma region Print Config

			void print_config(const std::vector <std::string> & config, size_t index, std::ostringstream & oss, int & cols, int & row) {
				int width = 1; if (config.size() > 9) width = 2; else if (config.size() > 99) width = 3; else if (config.size() > 999) width = 4;

				while (++row < total_rows - 3) {
					int length = 0; std::string temp = "";
					std::ostringstream ss; ss << Y " " << std::left << std::setw(width) << std::setfill(' ') << index << NC;

					if (index < config.size()) temp = ss.str() + "  " + config[index]; index++;
					if (temp.empty()) { oss << C "│"; setLine(C, " ", cols + 2, oss); oss << "│" NC << std::endl; continue; }
					length = Utils::str_nocolor_length(temp);
					if (length > cols + 2) temp = Utils::str_nocolor_trunc(temp, cols - 1);
					length = (cols + 2) - Utils::str_nocolor_length(temp);
					if (length < 0) length = 0;
					oss << C "│" NC << temp;
					setLine(C, " ", length, oss);
					oss << C "│" NC << std::endl;
				}
			}

		#pragma endregion

		#pragma region Print Buttons

			void print_buttons(std::ostringstream & oss, int & cols, int & row) {
			//	MAIN
				std::string Color1 = G UN; std::string Color2 = NC Y;
				std::string top = C "├──────┬";
				std::string middle = C "│ " + Color1 + "E" + Color2 + "xit " C "│";
				std::string bottom = C "└──────┴";
				int	length = (cols + 2) - 7;

				if (Settings::vserver.size() > 0) {
					top += C "─────────┬";
					middle += " " + Color1 + "W" + Color2 + "ebServ " C "│";
					bottom +=  "─────────┴";
					length -= 10;
				}

				if (Settings::current_vserver == -1) {
					if (Settings::global.config_displayed == false) {
						top += C "──────────┬";
						middle += " " + Color1 + "S" + Color2 + "ettings " C "│";
						bottom +=  "──────────┴";
						length -= 11;
						if (Settings::global.log.both.size() > 0) {
							top += C "───────────┬";
							middle += " " + Color1 + "C" + Color2 + "lear log " C "│";
							bottom +=  "───────────┴";
							length -= 12;
						}
					} else {
						top += C "─────┬";
						middle += " " + Color1 + "L" + Color2 + "og " C "│";
						bottom +=  "─────┴";
						length -= 6;
					}
				}
			//	V-SERVER
				if (Settings::current_vserver != -1) {
					if (Settings::global.status) {
						top += C "─────────┬";
						middle += " " + Color1 + "V" + Color2 + "server " C "│";
						bottom +=  "─────────┴";
						length -= 10;
					}
					if (Settings::vserver[Settings::current_vserver].config_displayed == false) {
						top += C "──────────┬";
						middle += " " + Color1 + "S" + Color2 + "ettings " C "│";
						bottom +=  "──────────┴";
						length -= 11;
						if (Settings::vserver[Settings::current_vserver].log.both.size() > 0) {
							top += C "───────────┬";
							middle += " " + Color1 + "C" + Color2 + "lear log " C "│";
							bottom +=  "───────────┴";
							length -= 12;
						}
					} else {
						top += C "─────┬";
						middle += " " + Color1 + "L" + Color2 + "og " C "│";
						bottom +=  "─────┴";
						length -= 6;
					}
				}

			//	DATA
				std::string SData1, SData2, Data1, Data2, Conect;
				if (Settings::current_vserver == -1) {
					SData1 = "0.00 MB/s";
					SData2 = "0.00 MB/s";
					Data1 = Y "0.00 " C "MB/s";
					Data2 = Y "0.00 " C "MB/s";
					Conect = "0";
				} else if (Settings::current_vserver != -1) {
					SData1 = "0.00 MB/s";
					SData2 = "0.00 MB/s";
					Data1 = Y "0.00 " C "MB/s";
					Data2 = Y "0.00 " C "MB/s";
					Conect = "0";
				}

			//	PRINT BUTTONS
				oss << top;
				if (length >= static_cast<int>((SData1.size() + SData2.size() + Conect.size() + 15))) {
					setLine(C, "─", length - 15 - (SData1.size() + SData2.size() + Conect.size() ), oss);
					oss << C "┬" NC; setLine(C, "─", Conect.size() + 4, oss);
					oss << C "┬" NC; setLine(C, "─", SData1.size() + 4, oss);
					oss << C "┬" NC; setLine(C, "─", SData2.size() + 4, oss); oss << "┤" NC << std::endl; row++;
				} else {
					setLine(C, "─", length, oss); oss << "┤" NC << std::endl; row++;
				}
				oss << middle;
				if (length >= static_cast<int>((SData1.size() + SData2.size() + Conect.size() + 15)))  {
					setLine(NC, " ", length - 15 - (SData1.size() + SData2.size() + Conect.size()), oss);
					oss << C "│ " G "Ϟ " Y << Conect << C " │ " G "↑ " C << Data1 << C " │ " G "↓ " C << Data2 << C " │" NC << std::endl; row++;
				} else {
					setLine(NC, " ", length, oss); oss << C "│" NC << std::endl; row++;
				}
				oss << bottom;
				if (length >= static_cast<int>((SData1.size() + SData2.size() + Conect.size() + 15)))  {
					setLine(C, "─", length - 15 - (SData1.size() + SData2.size() + Conect.size()), oss);
					oss << C "┴" NC; setLine(C, "─", Conect.size() + 4, oss);
					oss << C "┴" NC; setLine(C, "─", SData1.size() + 4, oss);
					oss << C "┴" NC; setLine(C, "─", SData2.size() + 4, oss); oss << "┘" NC << std::endl; row++;
				} else {
					setLine(C, "─", length, oss); oss << "┘" NC << std::endl; row++;
				}
			}

		#pragma endregion

		#pragma region Output

			void Display::Output() {
				if (Settings::check_only || RawModeDisabled || ForceRawModeDisabled) return;
				if (drawing) return; else drawing = true;																						//	┌───┐ ◄ ►
				winsize w; ioctl(0, TIOCGWINSZ, &w); int cols = w.ws_col - 4, row = 0;															//	├─┬─┤  ▲
				total_cols = cols; total_rows = w.ws_row; log_rows = total_rows - 9;															//	├─┴─┤  ▼
				std::ostringstream oss; std::ostringstream ss; ss << Settings::vserver.size(); std::string temp = ss.str();						//	├─┬─┤  █
				std::string Status; std::string Color = RD; std::string LArrow = "◄ "; std::string RArrow = "► ";								//	├─┼─┤
				if (Settings::vserver.size() > 0) Color = G;																					//	├─┴─┤  ▒
				if (Settings::global.status) Status = G; else Status = RD;																				//	└───┘ ↑ ↓
				if (Settings::vserver.size() == 0) { LArrow = "  "; RArrow = "  "; }
				
			//	TITLE
				oss << CHIDE CUU;
				oss << C "┌─────────────────┬"; setLine(C, "─", (cols + 2) - 18, oss); oss << "┐" NC << std::endl; row++;
				oss << C "│ V-Servers: " << Color << temp; setLine(C, " ", 5 - temp.size(), oss); oss << C "│"; setPadding("WEBSERV 1.0", Status, " ", (cols + 2) - 20, 1, oss); oss << RD "X " C "│" NC << std::endl; row++;
				oss << C "├─────────────────┤"; setLine(Status, "▄", (cols + 2) - 18, oss); oss << C "│" NC << std::endl; row++;

			//	COLOR LINES
				bool some = false;
				if (Settings::vserver.size() > 0 && Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].status) Status = G;
				else if (Settings::vserver.size() > 0 && Settings::current_vserver == -1) { Status = RD;
					for (size_t i = 0; i < Settings::vserver.size(); ++i) {
						if (Settings::vserver[i].status) Status = G;
						else if (!Settings::vserver[i].status) some = true;
					}
				} else Status = RD;
				if (Settings::global.status == false) Status = RD;
				if (Status != RD && some) Status = Y;

			//	MEM & CPU
				temp = monitor.getMEMinStr();
				oss << C "│ MEM: " G << temp; setLine(C, " ", 11 - temp.size(), oss);  oss << C "│"; setLine(Status, "▀", (cols + 2) - 18, oss); oss << C "│" NC << std::endl; row++;
				temp = monitor.getCPUinStr();
				oss << C "│ CPU: " G << temp; setLine(C, " ", 11 - temp.size(), oss); oss << C "│ " Y << LArrow;

			//	NAME
				ss.str("");
				if (Settings::vserver.size() > 0 && Settings::current_vserver != -1) {
					if (Settings::vserver[Settings::current_vserver].get("server_name").empty() && Settings::current_vserver == 0) temp = "(1) Default";
					else if (Settings::vserver[Settings::current_vserver].get("server_name").empty()) {
						ss << Settings::current_vserver + 1;
						temp = "(" + ss.str() + ") V-Server";
					} else {
						ss << Settings::current_vserver + 1;
						temp = "(" + ss.str() + ") " + Settings::vserver[Settings::current_vserver].get("server_name");
					}
				} else if (Status == RD && Settings::vserver.size() > 0)
					temp = "Virtual servers offline";
				else if (Settings::vserver.size() > 0 && Settings::current_vserver == -1)
					if (some)
						temp = "Some virtual servers online";
					else
						temp = "Virtual servers online";
				else
					temp = "No virtual servers available";

				if (temp.size() > static_cast<size_t>((cols + 2) - 27)) temp = temp.substr(0, (cols + 2) - 30) + "...";
				setPadding(temp, Status, " ", (cols + 2) - 24, 1, oss);
				oss << " " Y << RArrow << C "│" NC << std::endl; row++;
				oss << C "├─────────────────┴"; setLine(C, "─", (cols + 2) - 18, oss); oss << "┤" NC << std::endl; row++;

			//	LOG & SETTINGS
				if (Settings::current_vserver == -1 && Settings::global.config_displayed == false) {
					if (Settings::global.autolog) {
						if (static_cast<int>(Settings::global.log.both.size()) - (log_rows - 1) < 0) Settings::global.log_index = 0;
						else Settings::global.log_index = static_cast<int>(Settings::global.log.both.size()) - (log_rows - 1);
					}
					print_log(Settings::global.log.both, Settings::global.log_index, oss, cols, row);
				} else if (Settings::current_vserver == -1 && Settings::global.config_displayed == true)
					print_config(Settings::global.config, Settings::global.config_index, oss, cols, row);
				if (Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == false) {
					if (Settings::vserver[Settings::current_vserver].autolog) {
						if (static_cast<int>(Settings::vserver[Settings::current_vserver].log.both.size()) - (log_rows - 1) < 0) Settings::vserver[Settings::current_vserver].log_index = 0;
						else Settings::vserver[Settings::current_vserver].log_index = static_cast<int>(Settings::vserver[Settings::current_vserver].log.both.size()) - (log_rows - 1);
					}
					print_log(Settings::vserver[Settings::current_vserver].log.both, Settings::vserver[Settings::current_vserver].log_index, oss, cols, row);
				} else if (Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == true)
					print_config(Settings::vserver[Settings::current_vserver].config, Settings::vserver[Settings::current_vserver].config_index, oss, cols, row);
			// BUTTONS
				print_buttons(oss, cols, row);

				if (redraw) { drawing = false; redraw = false; std::cout.flush(); std::cout.clear(); failCount = 0; Output(); return; }
				std::cout << oss.str(); std::cout.flush();
				if (std::cout.fail()) {
					std::cout.clear(); drawing = false; failCount++;
					if (failCount < maxFails) { Output(); return; }
				}
				failCount = 0;
				drawing = false;
			}

		#pragma endregion

	#pragma endregion

	#pragma region Logo

		void Display::Logo() {
			std::cout << C	<< std::endl << std::endl
					  << 	" ██╗    ██╗███████╗██████╗ ███████╗███████╗██████╗ ██╗   ██╗" 	<< std::endl
					  << 	" ██║    ██║██╔════╝██╔══██╗██╔════╝██╔════╝██╔══██╗██║   ██║" << std::endl
					  << 	" ██║ █╗ ██║█████╗  ██████╔╝███████╗█████╗  ██████╔╝██║   ██║" << std::endl
					  << 	" ██║███╗██║██╔══╝  ██╔══██╗╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝" << std::endl
					  << 	" ╚███╔███╔╝███████╗██████╔╝███████║███████╗██║  ██║ ╚████╔╝ " << std::endl
					  << 	"  ╚══╝╚══╝ ╚══════╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  " << std::endl
					  << NC	<< std::endl;
		}

	#pragma endregion

#pragma endregion
