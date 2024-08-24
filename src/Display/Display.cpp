/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 14:37:32 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/24 15:28:42 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Monitor.hpp"
#include "Settings.hpp"
#include "Net.hpp"
#include "Thread.hpp"

#pragma region Variables

	pthread_mutex_t			Display::mutex;
	
	bool					Display::drawing = false;													//	Is in the middle of an Output()
	int						Display::failCount = 0;														//	Current number of fails when printing in the the terminal
	int						Display::maxFails = 3;														//	Maximum numbers of retries to print in the terminal if something fails
	bool					Display::RawModeDisabled = true;											//	Status of the terminal (false if in raw mode)
	bool					Display::ForceRawModeDisabled = false;										//	Force terminal in normal mode (not raw mode)
	bool					Display::Resized = false;													//	True if the terminal has been resized
	bool					Display::redraw = false;													//	Is in the middle of an Output()

	pthread_t				Display::_thread;
	bool					Display::_terminate = false;												//	Flag the thread to finish
	bool					Display::_update;															//	Flag for a redraw in the next iteration
	bool					Display::_logo;																//	Flag for printing the logo

	const int				Display::UPDATE_INTERVAL = 10;												//	Interval in miliseconds for the thread main loop

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
		if (Settings::check_only || !Display::isRawMode() || Display::ForceRawModeDisabled) std::cout << CL CL "  ";
		Thread::set_int(Display::mutex, Settings::terminate, signum + 128);
	}

	static void resizeHandler(int signum) {
		(void) signum;
		Thread::set_bool(Display::mutex, Display::Resized, true);
	}

	static void stopHandler(int signum) {
		(void) signum;
		if (Settings::check_only || !Display::isRawMode() || Display::ForceRawModeDisabled) std::cout << CL CL "  ";
		Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
		Display::disableRawMode();
		Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);
		raise(SIGSTOP);
	}

    static void resumeHandler(int signum) {
        (void) signum;
		Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
        Display::enableRawMode();
		Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);
		if (Settings::check_only || !Display::isRawMode() || Display::ForceRawModeDisabled) Display::logo();
    }

	static void quitHandler(int signum) {
		if (Settings::check_only || !Display::isRawMode() || Display::ForceRawModeDisabled) std::cout << CL CL "  ";
		Thread::set_int(Display::mutex, Settings::terminate, signum + 128);
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


#pragma region Input

	#pragma region Keys

		#pragma region Left

			static void Left() {
				if (Settings::vserver.size() == 0)															return;

				if (Settings::current_vserver == -1)														Settings::current_vserver = static_cast<int>(Settings::vserver.size() - 1);
				else																						Settings::current_vserver--;

				if (Display::drawing) { Display::redraw = true; return; } Display::Output();
			}

		#pragma endregion

		#pragma region Right

			static void Right() {
				if (Settings::vserver.size() == 0)															return;

				if (Settings::current_vserver == static_cast<int>(Settings::vserver.size() - 1))			Settings::current_vserver = -1;
				else																						Settings::current_vserver++;

				if (Display::drawing) { Display::redraw = true; return; } Display::Output();
			}

		#pragma endregion

		#pragma region Up

			static void Up() {
				VServer * VServ;
				if 		(Settings::current_vserver == -1)													VServ = &Settings::global;
				else 																						VServ = &Settings::vserver[Settings::current_vserver];

				if 		(VServ->config_displayed == false && VServ->log_index > 0) {						VServ->log_index--; VServ->autolog = false; }
				else if (VServ->config_displayed == true && VServ->config_index > 0)						VServ->config_index--;

				if (Display::drawing) { Display::redraw = true; return; } Display::Output();
			}

		#pragma endregion

		#pragma region Down

			static void Down() {
				VServer * VServ;
				if 		(Settings::current_vserver == -1)													VServ = &Settings::global;
				else 																						VServ = &Settings::vserver[Settings::current_vserver];

				Thread::mutex_set(Log::mutex, Thread::MTX_LOCK);

				if		(VServ->config_displayed == false
						&& static_cast<int>(VServ->log.both.size()) >= log_rows
						&& static_cast<int>(VServ->log.both.size()) - (log_rows - 1) > static_cast<int>(VServ->log_index)) {	if (++VServ->log_index == VServ->log.both.size() - (log_rows - 1)) VServ->autolog = true; }

				else if (VServ->config_displayed == true
						&& static_cast<int>(VServ->config.size()) >= log_rows
						&& static_cast<int>(VServ->config.size()) - (log_rows - 1) > static_cast<int>(VServ->config_index))		Settings::global.config_index++;

				Thread::mutex_set(Log::mutex, Thread::MTX_UNLOCK);


				if (Display::drawing) { Display::redraw = true; return; } Display::Output();
			}

		#pragma endregion

		#pragma region Home

			static void Home() {
				VServer * VServ;
				if 		(Settings::current_vserver == -1)													VServ = &Settings::global;
				else 																						VServ = &Settings::vserver[Settings::current_vserver];
		
				if		(VServ->config_displayed == false && VServ->log_index > 0) {						VServ->log_index = 0; VServ->autolog = false; }
				else if ( VServ->config_displayed == true && VServ->config_index > 0)						VServ->config_index = 0;

				if (Display::drawing) { Display::redraw = true; return; } Display::Output();
			}

		#pragma endregion

		#pragma region End

			static void End() {
				VServer * VServ; size_t temp = 0;
				if 		(Settings::current_vserver == -1)													VServ = &Settings::global;
				else 																						VServ = &Settings::vserver[Settings::current_vserver];

				Thread::mutex_set(Log::mutex, Thread::MTX_LOCK);

				if (VServ->config_displayed == false && VServ->log.both.size() > 0 && VServ->log_index != VServ->log.both.size() - (log_rows - 1)) {

					if (static_cast<int>(VServ->log.both.size()) - (log_rows - 1) < 0)						temp = 0;
					else 																					temp = static_cast<int>(VServ->log.both.size()) - (log_rows - 1);

					if (VServ->log_index == temp) {															Thread::mutex_set(Log::mutex, Thread::MTX_UNLOCK); return; }
					else																					VServ->log_index = temp;

					if (static_cast<int>(VServ->log_index) == static_cast<int>(VServ->log.both.size()) - (log_rows - 1))	VServ->autolog = true;

				} else if (VServ->config_displayed == true && VServ->config.size() > 0) {

					if (static_cast<int>(VServ->config.size()) - (log_rows - 1) < 0)						temp = 0;
					else																					temp = static_cast<int>(VServ->config.size()) - (log_rows - 1);

					if (VServ->config_index == temp) {														Thread::mutex_set(Log::mutex, Thread::MTX_UNLOCK); return; }
					else																					VServ->config_index = temp;
				}

				Thread::mutex_set(Log::mutex, Thread::MTX_UNLOCK);

				if (Display::drawing) { Display::redraw = true; return; } Display::Output();
			}

		#pragma endregion

		#pragma region Key_W

			static void Key_W() {
				if (Settings::vserver.size() == 0)															return;

				Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

					Settings::global.status = !Settings::global.status;

					if (Settings::global.status)															Net::ask_socket_create_all = true;
					else																					Net::ask_socket_close_all = true;

				Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

				Display::Output();
			}

		#pragma endregion

		#pragma region Key_V

			static void Key_V() {
				if (Settings::vserver.size() == 0 || Settings::current_vserver == -1) 						return;

				VServer * VServ = &Settings::vserver[Settings::current_vserver];

				Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

					VServ->force_off = !VServ->force_off;

					if		(VServ->status)																	Net::socket_action_list.push_back(std::make_pair(VServ, Net::CLOSE));
					else if (VServ->force_off == false)														Net::socket_action_list.push_back(std::make_pair(VServ, Net::CREATE));

				Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

				Display::Output();
			}

		#pragma endregion

		#pragma region Key_C

			static void Key_C() {
				VServer * VServ;
				if 		(Settings::current_vserver == -1)													VServ = &Settings::global;
				else 																						VServ = &Settings::vserver[Settings::current_vserver];
			
				Thread::mutex_set(Log::mutex, Thread::MTX_LOCK);
				if (VServ->log.both.size() > 0)																VServ->log.clear();
				Thread::mutex_set(Log::mutex, Thread::MTX_UNLOCK);

				Display::Output();
			}

		#pragma endregion

		#pragma region Key_L

			static void Key_L() {
				VServer * VServ;
				if 		(Settings::current_vserver == -1)													VServ = &Settings::global;
				else 																						VServ = &Settings::vserver[Settings::current_vserver];

				if (VServ->config_displayed == true)														VServ->config_displayed = false;

				Display::Output();
			}

		#pragma endregion

		#pragma region Key_S

			static void Key_S() {
				VServer * VServ;
				if 		(Settings::current_vserver == -1)													VServ = &Settings::global;
				else 																						VServ = &Settings::vserver[Settings::current_vserver];
				
				if (VServ->config_displayed == false)														VServ->config_displayed = true;

				Display::Output();
			}

		#pragma endregion

	#pragma endregion

	#pragma region Input

		void Display::Input() {
			if (isRawMode() == false) return;

			char c, seq[2];
			if (read(STDIN_FILENO, &c, 1) != 1) return ;																				//	This is Non-Blocking
			if (c == '\033' && read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1 && seq[0] == '[') {
				if (seq[1] == 'D')		Left();
				if (seq[1] == 'C')		Right();
				if (seq[1] == 'A') 		Up();
				if (seq[1] == 'B')		Down();
				if (seq[1] == 'H')		Home();
				if (seq[1] == 'F')		End();
			}
			if (c == 'w' || c == 'W')	Key_W();																						//	(S)tart / (S)top
			if (c == 'v' || c == 'V')	Key_V();																						//	(V)server start
			if (c == 'c' || c == 'C')	Key_C();																						//	(C)lear log
			if (c == 'l' || c == 'L')	Key_L();																						//	(L)og
			if (c == 's' || c == 'S')	Key_S();																						//	(S)ettings
			if (c == 'e' || c == 'E')	Thread::set_int(mutex, Settings::terminate, 0);												//	(E)xit
			if (c == 'r' || c == 'R') { std::cout << CB CHIDE CS CUU; std::cout.clear();  drawing = false; failCount = 0; Output(); }	//	(R)eset terminal
		}

	#pragma endregion

#pragma endregion

#pragma region Output

	#pragma region Output

		#pragma region Print Log

			void print_log(const std::deque<std::string> & log, size_t index, std::ostringstream &oss, int &cols, int &row) {
				while (++row < total_rows - 3) {
					int length = 0; std::string temp = "";
					if (index < log.size()) temp = log[index++];
					if (temp.empty()) { oss << C "│"; setLine(C, " ", cols + 2, oss); oss << C "│" NC << "\n"; continue; }
					length = Utils::str_nocolor_length(temp);
					if (length > cols + 2) temp = Utils::str_nocolor_trunc(temp, cols - 1);
					length = (cols + 2) - Utils::str_nocolor_length(temp);
					if (length < 0) length = 0;
					oss << C "│" NC << temp;
					setLine(C, " ", length, oss);
					oss << C "│" NC << "\n";
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
					if (temp.empty()) { oss << C "│"; setLine(C, " ", cols + 2, oss); oss << "│" NC << "\n"; continue; }
					length = Utils::str_nocolor_length(temp);
					if (length > cols + 2) temp = Utils::str_nocolor_trunc(temp, cols - 1);
					length = (cols + 2) - Utils::str_nocolor_length(temp);
					if (length < 0) length = 0;
					oss << C "│" NC << temp;
					setLine(C, " ", length, oss);
					oss << C "│" NC << "\n";
				}
			}

		#pragma endregion

		#pragma region Print Buttons

			void print_buttons(std::ostringstream & oss, int & cols, int & row) {
			//	MAIN
				Thread::mutex_set(Log::mutex, Thread::MTX_LOCK);
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
				Thread::mutex_set(Log::mutex, Thread::MTX_UNLOCK);

			//	DATA
				Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);
				std::string data1, data2, Downloaded, Uploaded, Conect;
				int Downloaded_size, Uploaded_size;
				Utils::formatSize(Net::read_bytes, data1, data2);
				Downloaded = Y + data1 + " " C + data2 + NC;
				Downloaded_size = data1.size() + data2.size() + 1;

				Utils::formatSize(Net::write_bytes, data1, data2);
				Uploaded = Y + data1 + " " C + data2 + NC;
				Uploaded_size = data1.size() + data2.size() + 1;
				Conect = Utils::ltos(Net::total_clients);
				Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

			//	PRINT BUTTONS
				oss << top;
				if (length >= Downloaded_size + Uploaded_size +  static_cast<int>(Conect.size()) + 15) {
					setLine(C, "─", length - 15 - (Downloaded_size + Uploaded_size + Conect.size()), oss);
					oss << C "┬" NC; setLine(C, "─", Conect.size() + 4, oss);
					oss << C "┬" NC; setLine(C, "─", Downloaded_size + 4, oss);
					oss << C "┬" NC; setLine(C, "─", Uploaded_size + 4, oss); oss << "┤" NC << "\n"; row++;
				} else {
					setLine(C, "─", length, oss); oss << "┤" NC << "\n"; row++;
				}
				oss << middle;
				if (length >= Downloaded_size + Uploaded_size +  static_cast<int>(Conect.size()) + 15)  {
					setLine(NC, " ", length - 15 - (Downloaded_size + Uploaded_size + Conect.size()), oss);
					oss << C "│ " G "Ϟ " Y << Conect << C " │ " G "↓ " C << Downloaded << C " │ " G "↑ " C << Uploaded << C " │" NC << "\n"; row++;
				} else {
					setLine(NC, " ", length, oss); oss << C "│" NC << "\n"; row++;
				}
				oss << bottom;
				if (length >= Downloaded_size + Uploaded_size +  static_cast<int>(Conect.size()) + 15)  {
					setLine(C, "─", length - 15 - (Downloaded_size + Uploaded_size + Conect.size()), oss);
					oss << C "┴" NC; setLine(C, "─", Conect.size() + 4, oss);
					oss << C "┴" NC; setLine(C, "─", Downloaded_size + 4, oss);
					oss << C "┴" NC; setLine(C, "─", Uploaded_size + 4, oss); oss << "┘" NC << "\n"; row++;
				} else {
					setLine(C, "─", length, oss); oss << "┘" NC << "\n"; row++;
				}
			}

		#pragma endregion

		#pragma region Output

			void Display::Output() {
				if (drawing || Settings::check_only || !isRawMode() || ForceRawModeDisabled) return; else  drawing = true;

			//	VARIABLES
				winsize w; ioctl(0, TIOCGWINSZ, &w); int cols = w.ws_col - 4, row = 0;
				total_cols = cols; total_rows = w.ws_row; log_rows = total_rows - 9;

				std::ostringstream oss; std::ostringstream ss; ss << Settings::vserver.size(); std::string temp = ss.str();
				std::string Status = RD; std::string Color = RD; std::string LArrow = "  ", RArrow = "  ";

				if (Thread::get_bool(mutex, Settings::global.status))									Status = G;
				if (Settings::vserver.size() > 0)														Color = G;
				if (Settings::vserver.size() > 0) { 													LArrow = "◄ "; RArrow = "► "; }

			//	TITLE
				oss << CHIDE CUU;
				oss << C "┌─────────────────┬"; setLine(C, "─", (cols + 2) - 18, oss); oss << "┐" NC << "\n"; row++;
				oss << C "│ V-Servers: " << Color << temp; setLine(C, " ", 5 - temp.size(), oss); oss << C "│  "; setPadding("WEBSERV 1.0", Status, " ", (cols + 2) - 22, 1, oss); oss << RD "X " C "│" NC << "\n"; row++;
				oss << C "├─────────────────┤"; setLine(Status, "▄", (cols + 2) - 18, oss); oss << C "│" NC << "\n"; row++;

			//	COLOR LINES
				bool some = false;
				if (Settings::vserver.size() > 0 && Settings::current_vserver != -1 && Thread::get_bool(mutex, Settings::vserver[Settings::current_vserver].status))	Status = G;
				else if (Settings::vserver.size() > 0 && Settings::current_vserver == -1) {													Status = RD;
					for (size_t i = 0; i < Settings::vserver.size(); ++i) {
						if (Thread::get_bool(mutex, Settings::vserver[i].status))															Status = G;
						else if (!Thread::get_bool(mutex, Settings::vserver[i].status))													some = true;
					}
				} else 																														Status = RD;
				if (Thread::get_bool(mutex, Settings::global.status) == false)																Status = RD;
				if (Status != RD && some)																									Status = Y;

			//	MEM & CPU
				temp = monitor.getMEMinStr();
				oss << C "│ MEM: " G << temp; setLine(C, " ", 11 - temp.size(), oss);  oss << C "│"; setLine(Status, "▀", (cols + 2) - 18, oss); oss << C "│" NC << "\n"; row++;
				temp = monitor.getCPUinStr();
				oss << C "│ CPU: " G << temp; setLine(C, " ", 11 - temp.size(), oss); oss << C "│ " Y << LArrow;

			//	NAME
				ss.str("");
				if (Settings::vserver.size() > 0 && Settings::current_vserver != -1) {
					if (Settings::vserver[Settings::current_vserver].get("server_name").empty() && Settings::current_vserver == 0)	temp = "(1) Default";
					else if (Settings::vserver[Settings::current_vserver].get("server_name").empty()) {								ss << Settings::current_vserver + 1;
																																	temp = "(" + ss.str() + ") V-Server";
					} else {																										ss << Settings::current_vserver + 1;
																																	temp = "(" + ss.str() + ") " + Settings::vserver[Settings::current_vserver].get("server_name");
					}
				} else if (Status == RD && Settings::vserver.size() > 0)															temp = "Virtual servers offline";
				else if (Settings::vserver.size() > 0 && Settings::current_vserver == -1)
					if (some)																										temp = "Some virtual servers online";
					else																											temp = "Virtual servers online";
				else																												temp = "No virtual servers available";

				if (temp.size() > static_cast<size_t>((cols + 2) - 27)) 															temp = temp.substr(0, (cols + 2) - 30) + "...";
				setPadding(temp, Status, " ", (cols + 2) - 24, 1, oss); oss << " " Y << RArrow << C "│" NC << "\n"; row++;
				oss << C "├─────────────────┴"; setLine(C, "─", (cols + 2) - 18, oss); oss << "┤" NC << "\n"; row++;

			//	LOG & SETTINGS
				Thread::mutex_set(Log::mutex, Thread::MTX_LOCK);
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
				Thread::mutex_set(Log::mutex, Thread::MTX_UNLOCK);
			//	BUTTONS
				print_buttons(oss, cols, row);

			//	PRINT
				if (redraw) { drawing = false; redraw = false; std::cout.flush(); std::cout.clear(); failCount = 0; Output(); return; }
				std::cout << oss.str(); std::cout.flush();
				if (std::cout.fail()) {
					std::cout.clear(); drawing = false; failCount++;
					if (failCount < maxFails) { Output(); return; }
				}
				failCount = 0;
				drawing = false;
				Thread::set_bool(mutex, _update, false);
			}

		#pragma endregion

	#pragma endregion

	#pragma region Logo

		void Display::Logo() {
			std::cout << C	"\n\n"
					  <<	"\t ██╗    ██╗███████╗██████╗ ███████╗███████╗██████╗ ██╗   ██╗" 	<< "\n"
					  <<	"\t ██║    ██║██╔════╝██╔══██╗██╔════╝██╔════╝██╔══██╗██║   ██║" << "\n"
					  <<	"\t ██║ █╗ ██║█████╗  ██████╔╝███████╗█████╗  ██████╔╝██║   ██║" << "\n"
					  <<	"\t ██║███╗██║██╔══╝  ██╔══██╗╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝" << "\n"
					  <<	"\t ╚███╔███╔╝███████╗██████╔╝███████║███████╗██║  ██║ ╚████╔╝ " << "\n"
					  <<	"\t  ╚══╝╚══╝ ╚══════╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  " << "\n"
					  << NC "\n";
			Thread::set_bool(mutex, _logo, false);
		}

	#pragma endregion

	#pragma region Updaters

		void	Display::update() {
			Thread::set_bool(mutex, _update, true);
		}

		void	Display::logo() {
			Thread::set_bool(mutex, _logo, true);
		}

		int		Display::isTerminate() {
			return (Thread::get_int(mutex, Settings::terminate));
		}

		bool	Display::isRawMode() {
			return (!Thread::get_bool(mutex, RawModeDisabled));
		}

	#pragma endregion

#pragma endregion


#pragma region Thread

	#pragma region Main

		void * Display::main(void * args) { (void) args;
			while (Thread::get_bool(mutex, _terminate) == false) {
				Input();
				if (Resized) { if (drawing) redraw = true; else Output(); }
				if (Thread::get_bool(mutex, _logo)) Logo();
				if (Thread::get_bool(mutex, _update)) Output();
				usleep(UPDATE_INTERVAL * 1000);
			}

			if (!Settings::check_only && isRawMode() && !ForceRawModeDisabled) {
				usleep(100000); std::cout.flush(); std::cout.clear(); maxFails = 10; failCount = 0; drawing = false;
				Log::log(G "WebServ 1.0 closed successfully", Log::MEM_ACCESS); Log::process_logs(); Output();
			} else std::cout << CSHOW << std::endl;
			return (NULL);
		}

	#pragma endregion

	#pragma region Start

		void Display::start() {
			Thread::set_bool(mutex, _terminate, false);
			Thread::mutex_set(mutex, Thread::MTX_INIT);
			Thread::thread_set(_thread, Thread::THRD_CREATE, main);
		}

	#pragma endregion

	#pragma region Stop

		void Display::stop() {
			Thread::set_bool(mutex, _terminate, true);
			Thread::thread_set(_thread, Thread::THRD_JOIN);
			Thread::mutex_set(mutex, Thread::MTX_DESTROY);
		}

	#pragma endregion

#pragma endregion
