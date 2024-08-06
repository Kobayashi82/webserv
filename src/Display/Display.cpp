/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 14:37:32 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/06 02:22:00 by vzurera-         ###   ########.fr       */
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
			if (read(STDIN_FILENO, &c, 1) != 1) return ;																				//	This is Non-Blocking
			if (c == '\033' && read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1 && seq[0] == '[') {
                if (Settings::vserver.size() > 0 && seq[1] == 'D') {																	//	Right arrow
					if (Settings::current_vserver == -1)
						Settings::current_vserver = static_cast<int>(Settings::vserver.size() - 1);
					else
						Settings::current_vserver--;
					Output();
                } else if (Settings::vserver.size() > 0 && seq[1] == 'C') {																//	Left arrow
					if (Settings::current_vserver == static_cast<int>(Settings::vserver.size() - 1))
						Settings::current_vserver = -1;
					else
						Settings::current_vserver++;
					Output();
                } else if (seq[1] == 'A') {																								//	Up arrow
					if (Settings::current_vserver == -1 && Settings::config_displayed == false
						&& Settings::log_index > 0) {
							Settings::log_index--; Settings::autolog = false;
					} else if (Settings::current_vserver == -1 && Settings::config_displayed == true
						&& Settings::config_index > 0)
							Settings::config_index--;
					else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::config_displayed == false
						&& Settings::vserver[Settings::current_vserver].log_index > 0) {
							Settings::vserver[Settings::current_vserver].log_index--;
							Settings::vserver[Settings::current_vserver].autolog = false;
					} else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::config_displayed == true
						&& Settings::vserver[Settings::current_vserver].config_index > 0)
							Settings::vserver[Settings::current_vserver].config_index--;
					else return ;
					Output();
                } else if (seq[1] == 'B') {																								//	Down arrow
					if (Settings::current_vserver == -1 && Settings::config_displayed == false
						&& static_cast<int>(Log::Both.size()) >= Display::log_rows
						&& static_cast<int>(Settings::log_index) < static_cast<int>(Log::Both.size()) - (Display::log_rows - 1)) {
							Settings::log_index++;
							if (Settings::log_index == Log::Both.size() - (Display::log_rows - 1)) Settings::autolog = true;
					} else if (Settings::current_vserver == -1 && Settings::config_displayed == true
						&& static_cast<int>(Settings::config.size()) >= Display::log_rows
						&& static_cast<int>(Settings::config_index) < static_cast<int>(Settings::config.size()) - (Display::log_rows - 1))
							Settings::config_index++;
					else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::config_displayed == false
						&& static_cast<int>(Settings::vserver[Settings::current_vserver].both.size()) >= Display::log_rows
						&& static_cast<int>(Settings::vserver[Settings::current_vserver].log_index) < static_cast<int>(Settings::vserver[Settings::current_vserver].both.size()) - (Display::log_rows - 1)) {
							Settings::vserver[Settings::current_vserver].log_index++;
							if (Settings::vserver[Settings::current_vserver].log_index == Settings::vserver[Settings::current_vserver].both.size() - (Display::log_rows - 1)) Settings::vserver[Settings::current_vserver].autolog = true;
					} else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::config_displayed == true
						&& static_cast<int>(Settings::vserver[Settings::current_vserver].config.size()) >= Display::log_rows
						&& static_cast<int>(Settings::vserver[Settings::current_vserver].config_index) < static_cast<int>(Settings::vserver[Settings::current_vserver].config.size()) - (Display::log_rows - 1))
							Settings::vserver[Settings::current_vserver].config_index++;
					else return ;
					Output();
                } else if (seq[1] == 'H') {																								//	Home
					if (Settings::current_vserver == -1 && Settings::config_displayed == false && Settings::log_index > 0) {
						Settings::log_index = 0;
						Settings::autolog = false;
					} else if (Settings::current_vserver == -1 && Settings::config_displayed == true && Settings::config_index > 0)
						Settings::config_index = 0;
					else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::config_displayed == false
						&& Settings::vserver[Settings::current_vserver].log_index > 0) {
							Settings::vserver[Settings::current_vserver].log_index = 0;
							Settings::vserver[Settings::current_vserver].autolog = false;
					} else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::config_displayed == true
						&& Settings::vserver[Settings::current_vserver].config_index > 0)
							Settings::vserver[Settings::current_vserver].config_index = 0;
					else return ;
					Output();
            	} else if (seq[1] == 'F') {																								//	End
					size_t temp = 0;
					if (Settings::current_vserver == -1 && Settings::config_displayed == false && Log::Both.size() > 0
						&& Settings::log_index != Log::Both.size() - (Display::log_rows - 1)) {
							if (static_cast<int>(Log::Both.size()) - (Display::log_rows - 1) < 0) temp = 0;
							else temp = static_cast<int>(Log::Both.size()) - (Display::log_rows - 1);
							if (Settings::log_index == temp) return ; else Settings::log_index = temp;
							if (static_cast<int>(Settings::log_index) == static_cast<int>(Log::Both.size()) - (Display::log_rows - 1)) Settings::autolog = true;
					} else if (Settings::current_vserver == -1 && Settings::config_displayed == true && Settings::config.size() > 0) {
						if (static_cast<int>(Settings::config.size()) - (Display::log_rows - 1) < 0) temp = 0;
						else temp = static_cast<int>(Settings::config.size()) - (Display::log_rows - 1);
						if (Settings::config_index == temp) return ; else Settings::config_index = temp;
					} else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::config_displayed == false
						&& Settings::vserver[Settings::current_vserver].both.size() > 0) {
							if (static_cast<int>(Settings::vserver[Settings::current_vserver].both.size()) - (Display::log_rows - 1) < 0) temp = 0;
							else temp = static_cast<int>(Settings::vserver[Settings::current_vserver].both.size()) - (Display::log_rows - 1);
							if (Settings::vserver[Settings::current_vserver].log_index == temp) return ; else Settings::vserver[Settings::current_vserver].log_index = temp;
							if (static_cast<int>(Settings::vserver[Settings::current_vserver].log_index) == static_cast<int>(Settings::vserver[Settings::current_vserver].both.size()) - (Display::log_rows - 1)) Settings::vserver[Settings::current_vserver].autolog = true;
					} else if (Settings::vserver.size() > 1 && Settings::current_vserver != -1 && Settings::config_displayed == true
						&& Settings::vserver[Settings::current_vserver].config.size() > 0) {
							if (static_cast<int>(Settings::vserver[Settings::current_vserver].config.size()) - (Display::log_rows - 1) < 0) temp = 0;
							else temp = static_cast<int>(Settings::vserver[Settings::current_vserver].config.size()) - (Display::log_rows - 1);
							if (Settings::vserver[Settings::current_vserver].config_index == temp) return ; else Settings::vserver[Settings::current_vserver].config_index = temp;
					} else return ;
					Output();
				}
            }
			if (c == 'w' && Settings::vserver.size() > 0) {																				//	(S)tart / (S)top
				Settings::status = !Settings::status;
				if (Settings::status)
					Log::log_access("WebServ 1.0 stoped");
				else
					Log::log_access("WebServ 1.0 started");
			} else if (c == 'v' && Settings::status && Settings::vserver.size() > 0
				&& Settings::current_vserver != -1) {																					//	(V)server start
					Settings::vserver[Settings::current_vserver].status = !Settings::vserver[Settings::current_vserver].status;
					if (Settings::vserver[Settings::current_vserver].status)
						Log::log_access("VServer started", &Settings::vserver[Settings::current_vserver]);
					else
						Log::log_access("VServer stoped", &Settings::vserver[Settings::current_vserver]);
			} else if (c == 'c') {																										//	(C)lear log
				if (Settings::current_vserver == -1 && Log::Both.size() > 0)
					Log::clear();
				else if (Settings::current_vserver != -1 && Settings::vserver.size() > 0
					&& Settings::vserver[Settings::current_vserver].both.size() > 0)
						Settings::vserver[Settings::current_vserver].clear_logs();
				else return ;
				Output();
			} else if (c == 'l') {																										//	(L)og
				if (Settings::current_vserver == -1 && Settings::config_displayed == true)
					Settings::config_displayed = false;
				else if (Settings::current_vserver <static_cast<int>(Settings::vserver.size() > 0)
					&& Settings::vserver[Settings::current_vserver].config_displayed == true)
						Settings::vserver[Settings::current_vserver].config_displayed = false;
				else return ;
				Output();
			} else if (c == 's') {																										//	(S)ettings
				if (Settings::current_vserver == -1 && Settings::config_displayed == false)
					Settings::config_displayed = true;
				else if (Settings::current_vserver <static_cast<int>(Settings::vserver.size() > 0)
					&& Settings::vserver[Settings::current_vserver].config_displayed == false)
						Settings::vserver[Settings::current_vserver].config_displayed = true;
				else return ;
				Output();
			} else if (c == 'e') { disableRawMode(); Settings::terminate = 0;															//	(E)xit
			} else if (c == 'r') { std::cout << CB CHIDE CS CUU; Output(); }															//	(R)eset terminal
		}

	#pragma endregion

	#pragma region Output

		#pragma region Print Log

		void print_log(const std::deque<std::string> & log, size_t index, std::ostringstream &oss, int &cols, int &row) {
			while (++row < Display::rows - 3) {
				std::string temp = ""; std::string isRD = "";
				if (index < log.size()) temp = log[index++];
				if (temp.empty()) { oss << C "│"; setLine(C, " ", cols + 2, oss); oss << C "│" NC << std::endl; continue; }
				if (temp.find(RD) == 0) isRD = RD;
				if (temp.size() - isRD.size() > static_cast<size_t>(cols + 2)) temp = temp.substr(0, isRD.size() + cols - 1) + "...";
				int length = (cols + 2) - static_cast<int>(temp.size() - isRD.size());
				if (length < 0) length = 0;
				oss << C "│" NC << temp;
				setLine(C, " ", length, oss);
				oss << C "│" NC << std::endl;
			}
		}

		#pragma endregion

		#pragma region Print Config

		void print_config(const std::vector <std::string> & config, size_t index, std::ostringstream & oss, int & cols, int & row) {
			while (++row < Display::rows - 3) {
					std::string temp = ""; std::string isRD = "";
					if (index < config.size()) temp = config[index++];
					if (temp.empty()) { oss << C "│"; setLine(C, " ", cols + 2, oss); oss << "│" NC << std::endl; continue; }
					if (temp.find(RD) == 0) isRD = RD;
					if (temp.size() - isRD.size() > static_cast<size_t>(cols + 2)) temp = temp.substr(0, isRD.size() + cols - 1) + "...";
					int length = (cols + 2) - static_cast<int>(temp.size() - isRD.size());
					if (length < 0) length = 0;
					oss << C "│" NC << temp;
					setLine(C, " ", length, oss);
					oss << "│" NC << std::endl;
				}
			}

		#pragma endregion

		#pragma region Output

			void Display::Output() {
				if (Display::drawing) return; else Display::drawing = true;																		//	┌───┐ ◄ ►
				winsize w; ioctl(0, TIOCGWINSZ, &w); int cols = w.ws_col - 4, row = 0;															//	├─┬─┤  ▲
				Display::cols = cols; Display::rows = w.ws_row; Display::log_rows = Display::rows - 9;											//	├─┴─┤  ▼
				std::ostringstream oss; std::ostringstream ss; ss << Settings::vserver.size(); std::string temp = ss.str();						//	├─┬─┤  █
				std::string Status; std::string Color = RD; std::string LArrow = "◄ "; std::string RArrow = "► ";								//	├─┼─┤
				if (Settings::vserver.size() > 0) Color = G;																					//	├─┴─┤  ▒
				if (Settings::status) Status = G; else Status = RD;																				//	└───┘ ↑ ↓
				if (Settings::vserver.size() == 0) { LArrow = "  "; RArrow = "  "; }
				
			//	TITLE
				oss << CHIDE CS CUU;
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
				if (Settings::status == false) Status = RD;
				if (Status != RD && some) Status = Y;

			//	MEM & CPU
				temp = monitor.get_memory_str();
				oss << C "│ MEM: " G << temp; setLine(C, " ", 11 - temp.size(), oss);  oss << C "│"; setLine(Status, "▀", (cols + 2) - 18, oss); oss << C "│" NC << std::endl; row++;
				temp = monitor.getCPUinStr();
				oss << C "│ CPU: " G << temp; setLine(C, " ", 11 - temp.size(), oss); oss << C "│ " Y << LArrow;

			//	NAME
				ss.str("");
				if (Settings::vserver.size() > 0 && Settings::current_vserver != -1) {
					if ((Settings::vserver[Settings::current_vserver].get("server_name").empty() || Settings::vserver[Settings::current_vserver].get("server_name") == "_") && Settings::current_vserver == 0) temp = "(1) Default";
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
				if (Settings::current_vserver == -1 && Settings::config_displayed == false) {
					if (Settings::autolog) {
						if (static_cast<int>(Log::Both.size()) - (Display::log_rows - 1) < 0) Settings::log_index = 0;
						else Settings::log_index = static_cast<int>(Log::Both.size()) - (Display::log_rows - 1);
					}
					print_log(Log::Both, Settings::log_index, oss, cols, row);
				} else if (Settings::current_vserver == -1 && Settings::config_displayed == true)
					print_config(Settings::config, Settings::config_index, oss, cols, row);
				if (Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == false) {
					if (Settings::vserver[Settings::current_vserver].autolog) {
						if (static_cast<int>(Settings::vserver[Settings::current_vserver].both.size()) - (Display::log_rows - 1) < 0) Settings::vserver[Settings::current_vserver].log_index = 0;
						else Settings::vserver[Settings::current_vserver].log_index = static_cast<int>(Settings::vserver[Settings::current_vserver].both.size()) - (Display::log_rows - 1);
					}
					print_log(Settings::vserver[Settings::current_vserver].both, Settings::vserver[Settings::current_vserver].log_index, oss, cols, row);
				} else if (Settings::current_vserver != -1 && Settings::vserver[Settings::current_vserver].config_displayed == true)
					print_config(Settings::vserver[Settings::current_vserver].config, Settings::vserver[Settings::current_vserver].config_index, oss, cols, row);

			//	BUTTONS (Main)
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
					if (Settings::config_displayed == false) {
						top += C "──────────┬";
						middle += " " + Color1 + "S" + Color2 + "ettings " C "│";
						bottom +=  "──────────┴";
						length -= 11;
						if (Log::Both.size() > 0) {
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
			//	BUTTONS (V-Server)
				if (Settings::current_vserver != -1) {
					if (Settings::status) {
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
						if (Settings::vserver[Settings::current_vserver].both.size() > 0) {
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

			//	BUTTONS ⚡
				std::string SData1, SData2, Data1, Data2;
				if (Settings::current_vserver == -1) {
					SData1 = "0.00 MB/s";
					SData2 = "0.00 MB/s";
					Data1 = Y "0.00 " C "MB/s";
					Data2 = Y "0.00 " C "MB/s";
				} else if (Settings::current_vserver != -1) {
					SData1 = "0.00 MB/s";
					SData2 = "0.00 MB/s";
					Data1 = Y "0.00 " C "MB/s";
					Data2 = Y "0.00 " C "MB/s";
				}

				oss << top;
				if (length >= static_cast<int>((SData1.size() + SData2.size() + 10))) {
					setLine(C, "─", length - 10 - (SData1.size() + SData2.size()), oss); oss << C "┬" NC; setLine(C, "─", SData1.size() + 4, oss); oss << C "┬" NC; setLine(C, "─", SData2.size() + 4, oss); oss << "┤" NC << std::endl; row++;
				} else {
					setLine(C, "─", length, oss); oss << "┤" NC << std::endl; row++;
				}
				oss << middle;
				if (length >= static_cast<int>((SData1.size() + SData2.size() + 10)))  {
					setLine(NC, " ", length - 10 - (SData1.size() + SData2.size()), oss);
					oss << C "│ " G "↑ " C << Data1 << C " │ " G "↓ " C << Data2 << C " │" NC << std::endl; row++;
				} else {
					setLine(NC, " ", length, oss); oss << C "│" NC << std::endl; row++;
				}
				oss << bottom;
				if (length >= static_cast<int>((SData1.size() + SData2.size() + 10)))  {
					setLine(C, "─", length - 10 - (SData1.size() + SData2.size()), oss); oss << C "┴" NC; setLine(C, "─", SData1.size() + 4, oss); oss << C "┴" NC; setLine(C, "─", SData2.size() + 4, oss); oss << "┘" NC << std::endl; row++;
				} else {
					setLine(C, "─", length, oss); oss << "┘" NC << std::endl; row++;
				}

				std::cout << oss.str();
				Display::drawing = false;
			}

		#pragma endregion

	#pragma endregion

#pragma endregion
