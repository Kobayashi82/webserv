/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Display.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/14 14:37:32 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/03 13:10:27 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Thread.hpp"
#include "Monitor.hpp"
#include "Settings.hpp"
#include "Socket.hpp"
#include "Communication.hpp"

#pragma region Variables

	pthread_mutex_t			Display::mutex;																//	Mutex for synchronizing access to shared resources
	
	bool					Display::drawing = false;													//	True if printing to the terminal
	int						Display::failCount = 0;														//	Current number of failures when printing to the terminal
	int						Display::maxFails = 3;														//	Maximum numbers of retries to print to the terminal if something fails
	bool					Display::RawModeDisabled = true;											//	Status of the terminal (false if in raw mode)
	bool					Display::ForceRawModeDisabled = false;										//	Force terminal in normal mode (not raw mode)
	bool					Display::Resized = false;													//	True if the terminal has been resized
	bool					Display::redraw = false;													//	Flags for a redraw of the terminal
	bool					Display::background = false;												//	True if the program is running in the background (&)
	int						Display::signal = 0;														//	Last signal code

	pthread_t				Display::_thread;
	bool					Display::_terminate = false;												//	Flags the thread to finish
	bool					Display::_update = false;													//	Flags for a redraw in the next iteration
	bool					Display::_logo = false;														//	Flags for printing the logo

	const int				Display::UPDATE_INTERVAL = 10;												//	Interval in milliseconds for the thread main loop

	static Monitor			monitor;																	//	Class to obtain MEM and CPU usage
	static struct termios	orig_termios;																//	Structure for terminal information
	static int				total_cols = 0;																//	Number of columns in the terminal
	static int				total_rows = 0;																//	Number of rows in the terminal
	static int				log_rows = 0;																//	Number of rows available for logs or settings

#pragma endregion

#pragma region Signals

	#pragma region Type

		static void killHandler(int signum)		{ Display::signal = 128 + signum; }
		static void quitHandler(int signum)		{ Display::signal = 128 + signum; }

		static void stopHandler(int signum)		{ Display::signal = 128 + signum; }
		static void resumeHandler(int signum)	{ Display::signal = 128 + signum; }

		static void resizeHandler(int signum)	{  Display::signal = 128 + signum; Thread::set_bool(Display::mutex, Display::Resized, (signum)); }

		static void childHandler(int signum)	{ Display::signal = 128 + signum;
		 	int saved_errno = errno;
			while (waitpid(-1, NULL, WNOHANG) > 0);
			errno = saved_errno;
		}

	#pragma endregion

	#pragma region Handler

		void Display::signal_handler() {
			std::signal(SIGINT, killHandler);
			std::signal(SIGQUIT, quitHandler);
			std::signal(SIGTSTP, stopHandler);
			std::signal(SIGCONT, resumeHandler);
			std::signal(SIGWINCH, resizeHandler);
			std::signal(SIGCHLD, childHandler);
		}

	#pragma endregion

	#pragma region Check

		int Display::signal_check() {	//	CTRL + 'C' or CTRL + '\'
			if (signal == 130 || signal == 131) {
				if (Settings::check_only || !isRawMode() || ForceRawModeDisabled) {
					std::cout << NC CL CL;
					if (!background) std::cout << " ";
				}

				return (1);
			}
			if (signal == 148) {		//	CTRL + 'Z'
				if (Settings::check_only || !isRawMode() || ForceRawModeDisabled) std::cout << NC CL CL "  ";

				Thread::mutex_set(mutex, Thread::MTX_LOCK);
					Display::background = true;
					disableRawMode();
				Thread::mutex_set(mutex, Thread::MTX_UNLOCK);
			}
			if (signal == 146) {		//	Resume (fg)

				Thread::mutex_set(mutex, Thread::MTX_LOCK);
					Display::background = false;
					enableRawMode();
				Thread::mutex_set(mutex, Thread::MTX_UNLOCK);

				if (!background && (Settings::check_only || !isRawMode() || ForceRawModeDisabled)) logo();
			}
			signal = 0;

			return (0);
		}

#pragma endregion

#pragma endregion

#pragma region Terminal

	#pragma region Raw Mode

		#pragma region Enable

			void Display::enableRawMode() {
				if (!background) std::cout << CHIDE;
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

					Thread::mutex_set(Log::mutex, Thread::MTX_LOCK);

						if 		(VServ->config_displayed == false && VServ->log_index > 1) {						VServ->log_index--; VServ->autolog = false; }
						else if (VServ->config_displayed == true && VServ->config_index > 0)						VServ->config_index--;

					Thread::mutex_set(Log::mutex, Thread::MTX_UNLOCK);

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
								&& static_cast<int>(VServ->config.size()) - (log_rows - 1) > static_cast<int>(VServ->config_index))		VServ->config_index++;

					Thread::mutex_set(Log::mutex, Thread::MTX_UNLOCK);

					if (Display::drawing) { Display::redraw = true; return; } Display::Output();
				}

			#pragma endregion

			#pragma region Home

				static void Home() {
					VServer * VServ;
					if 		(Settings::current_vserver == -1)													VServ = &Settings::global;
					else 																						VServ = &Settings::vserver[Settings::current_vserver];
			
					if		(VServ->config_displayed == false && VServ->log_index > 1) {						VServ->log_index = 1; VServ->autolog = false; }
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
					if (Settings::vserver.size() == 0 || Settings::global.bad_config) return;

					Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

						Settings::global.status = !Settings::global.status;

						if (Settings::global.status)															Socket::ask_socket = 1;
						else																					Socket::ask_socket = 2;

					Thread::mutex_set(Display::mutex, Thread::MTX_UNLOCK);

					Display::Output();
				}

			#pragma endregion

			#pragma region Key_V

				static void Key_V() {
					if (Settings::vserver.size() == 0 || Settings::current_vserver == -1 || Settings::vserver[Settings::current_vserver].bad_config || Settings::global.bad_config) return;

					VServer * VServ = &Settings::vserver[Settings::current_vserver];

					Thread::mutex_set(Display::mutex, Thread::MTX_LOCK);

						if (Settings::global.status) {
							VServ->force_off = !VServ->force_off;

							if		(VServ->status)																Socket::socket_action_list.push_back(std::make_pair(VServ, Socket::CLOSE));
							else if (VServ->force_off == false)													Socket::socket_action_list.push_back(std::make_pair(VServ, Socket::CREATE));
						}

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

			#pragma region Key_E

				static void Key_E() {
					for (std::deque<VServer>::iterator it = Settings::vserver.begin(); it != Settings::vserver.end(); ++it)
						Thread::set_bool(Display::mutex, it->force_off, true);
					Thread::set_bool(Display::mutex, Settings::global.status, false);
					Thread::set_int(Display::mutex, Settings::terminate, 0);
				}

			#pragma endregion

			#pragma region Key_R

				static void Key_R() {
					Thread::set_int(Display::mutex, Settings::terminate, 4);
				}

			#pragma endregion

			#pragma region Key_T

				static void Key_T() {
					std::cout << CB CHIDE CS CUU; std::cout.clear();
					Display::drawing = false; Display::failCount = 0; Display::Output();
				}

			#pragma endregion

		#pragma endregion

		#pragma region Input

			void Display::Input() {
				if (isRawMode() == false) return;

				char c, seq[4];
				if (read(STDIN_FILENO, &c, 1) != 1) return ;																							//	This is Non-Blocking
				if (c == '\033' && read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1 && seq[0] == '[') {
					if (seq[1] == 'D')		Left();
					if (seq[1] == 'C')		Right();
					if (seq[1] == 'A') 		Up();
					if (seq[1] == 'B')		Down();
					if (seq[1] == 'H')		Home();
					if (seq[1] == 'F')		End();
					return;
				}
				if (c == 'w' || c == 'W')	Key_W();																									//	(S)tart / (S)top
				if (c == 'v' || c == 'V')	Key_V();																									//	(V)server start
				if (c == 'c' || c == 'C')	Key_C();																									//	(C)lear log
				if (c == 'l' || c == 'L')	Key_L();																									//	(L)og
				if (c == 's' || c == 'S')	Key_S();																									//	(S)ettings
				if (c == 'e' || c == 'E')	Key_E();																									//	(E)xit
				if (c == 'r' || c == 'R')	Key_R();																									//	(R)eload configuration
				if (c == 't' || c == 'T')	Key_T();																									//	Reset (T)erminal
			}

		#pragma endregion

	#pragma endregion

	#pragma region Output

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

		#pragma region Print Log

			void print_log(const std::deque<std::string> & log, size_t index, std::ostringstream &oss, int &cols, int &row) {
				if (log.size() > 0 && index > 0) index--;

				while (++row < total_rows - 3) {
					int length = 0; std::string temp = "";
					if (index < log.size()) temp = log[index++];
					if (temp.empty()) { oss << C "│"; setLine(C, " ", cols + 2, oss); oss << C "│" NC << "\n"; continue; }
					length = Utils::str_nocolor_length(temp);
					if (length > cols + 2) temp = Utils::str_nocolor_trunc(temp, cols - 1);
					length = (cols + 2) - Utils::str_nocolor_length(temp);
					if (length < 0) length = 0;
					if (temp == "---") {
						if (index == log.size()) { oss << C "│"; setLine(C, " ", cols + 2, oss); oss << C "│" NC << "\n"; continue; }
						oss << C "├" NC;
						setLine(SKY900, "─", cols + 2, oss);
						oss << C "┤" NC << "\n";
					} else {
						oss << C "│" NC << temp;
						setLine(C, " ", length, oss);
						oss << C "│" NC << "\n";
					}
				}
			}

		#pragma endregion

		#pragma region Print Config

			void print_config(const std::vector <std::string> & config, size_t index, std::ostringstream & oss, int & cols, int & row) {
				int width = 1; if (config.size() > 9) width = 2; else if (config.size() > 99) width = 3; else if (config.size() > 999) width = 4;

				while (++row < total_rows - 3) {
					int length = 0; std::string temp = "";
					std::ostringstream ss; ss << Y " " << std::left << std::setw(width) << std::setfill(' ') << index << NC;

					if (index < config.size()) temp = ss.str() + "  " + Utils::replace_tabs(config[index++]);
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

					if (!Settings::global.bad_config && Settings::vserver.size() > 0) {
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
					int Downloaded_size, Uploaded_size; std::ostringstream ss;

					Utils::formatSize(Communication::read_bytes, data1, data2);
					ss << std::left << std::fixed << std::setprecision(2) << Utils::formatSizeDbl(Communication::read_bytes); data1 = ss.str();
					Downloaded = Y + data1 + " " C + data2 + NC;
					Downloaded_size = data1.size() + data2.size() + 1;

					Utils::formatSize(Communication::write_bytes, data1, data2);
					ss.str(""); ss << std::left << std::fixed << std::setprecision(2) << Utils::formatSizeDbl(Communication::write_bytes); data1 = ss.str();
					Uploaded = Y + data1 + " " C + data2 + NC;
					Uploaded_size = data1.size() + data2.size() + 1;

					ss.str(""); ss << std::left << std::setw(3) << Utils::ltos(Communication::total_clients);
					Conect = ss.str();
						
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
				std::string CGREEN = GREEN700, CRED = RED700, CYELLOW = ORANGE400;
				winsize w; ioctl(0, TIOCGWINSZ, &w); int cols = w.ws_col - 4, row = 0;
				total_cols = cols; total_rows = w.ws_row; log_rows = total_rows - 9;

				int active_servers = 0;
				for (std::deque<VServer>::iterator it = Settings::vserver.begin(); it != Settings::vserver.end(); ++it)
					if (Thread::get_bool(mutex, it->force_off) == false) active_servers++;
				std::ostringstream oss; std::ostringstream ss; ss << active_servers; std::string temp = ss.str();
				std::string Status = CRED; std::string Color = CRED; std::string LArrow = "  ", RArrow = "  ";

				if (Thread::get_bool(mutex, Settings::global.status))																		Status = CGREEN;
				if (active_servers > 0)																										Color  = CGREEN;
				if (Settings::vserver.size() > 0) { 																						LArrow = "◄ "; RArrow = "► "; }

			//	TITLE
				oss << CHIDE CUU;
				oss << C "┌─────────────────┬"; setLine(C, "─", (cols + 2) - 18, oss); oss << "┐" NC << "\n"; row++;
				oss << C "│ V-Servers: " << Color << temp; setLine(C, " ", 5 - temp.size(), oss); oss << C "│  "; setPadding(Utils::strToUpper(Settings::server_name) + " " + Settings::server_version, Status, " ", (cols + 2) - 22, 1, oss); oss << RD "X " C "│" NC << "\n"; row++;
				oss << C "├─────────────────┤"; setLine(Status, "▄", (cols + 2) - 18, oss); oss << C "│" NC << "\n"; row++;

			//	COLOR LINES
				bool some = false;
				if	(Settings::vserver.size() > 0 && Settings::current_vserver != -1
					&& Thread::get_bool(mutex, Settings::vserver[Settings::current_vserver].force_off) == false)							Status = CGREEN;
				else if (Settings::vserver.size() > 0 && Settings::current_vserver == -1) {													Status = CRED;
					for (size_t i = 0; i < Settings::vserver.size(); ++i) {
						if (Thread::get_bool(mutex, Settings::vserver[i].force_off) == false)												Status = CGREEN;
						else if (Thread::get_bool(mutex, Settings::vserver[i].force_off) == true)											some = true;
					}
				} else 																														Status = CRED;
				if (Thread::get_bool(mutex, Settings::global.status) == false)																Status = CRED;
				if (Status != CRED && some)																									Status = CYELLOW;

			//	MEM & CPU
				temp = monitor.getMEMinStr();
				oss << C "│ MEM: " G << temp; setLine(C, " ", 11 - temp.size(), oss);  oss << C "│"; setLine(Status, "▀", (cols + 2) - 18, oss); oss << C "│" NC << "\n"; row++;
				temp = monitor.getCPUinStr();
				oss << C "│ CPU: " G << temp; setLine(C, " ", 11 - temp.size(), oss); oss << C "│ " Y << LArrow;

			//	NAME
				ss.str("");
				if (Settings::vserver.size() > 0 && Settings::current_vserver != -1) {
					ss << Settings::current_vserver + 1; temp = "(" + ss.str() + ") ";
					if (Settings::vserver[Settings::current_vserver].get("server_name").empty())											temp += "Unnamed V-Server";
					else																													temp += Settings::vserver[Settings::current_vserver].get("server_name");
				} else if (Status == CRED && Settings::vserver.size() > 0)																	temp = "Virtual servers offline";
				else if (Settings::vserver.size() > 0 && Settings::current_vserver == -1)
					if (some)																												temp = "Some virtual servers online";
					else																													temp = "Virtual servers online";
				else																														temp = "No virtual servers available";

				if (temp.size() > static_cast<size_t>((cols + 2) - 27)) 																	temp = temp.substr(0, (cols + 2) - 30) + "...";
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
				setLine(C, " ", cols + 4, oss);

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

		#pragma region Logo

			void Display::Logo() {
				Thread::set_bool(mutex, _logo, false);
				if (background) return;
				std::cout << C	"\n\n"
						<<	"\t ██╗    ██╗███████╗██████╗ ███████╗███████╗██████╗ ██╗   ██╗" 	<< "\n"
						<<	"\t ██║    ██║██╔════╝██╔══██╗██╔════╝██╔════╝██╔══██╗██║   ██║" << "\n"
						<<	"\t ██║ █╗ ██║█████╗  ██████╔╝███████╗█████╗  ██████╔╝██║   ██║" << "\n"
						<<	"\t ██║███╗██║██╔══╝  ██╔══██╗╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝" << "\n"
						<<	"\t ╚███╔███╔╝███████╗██████╔╝███████║███████╗██║  ██║ ╚████╔╝ " << "\n"
						<<	"\t  ╚══╝╚══╝ ╚══════╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  " << "\n"
						<< NC "\n";
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Thread

	#pragma region Main

		void * Display::main(void * args) { (void) args;
			while (Thread::get_bool(mutex, _terminate) == false) {
				Input();
				if (Thread::get_bool(mutex, Resized) && isRawMode()) {
					Thread::set_bool(mutex, Resized, false);
					if (drawing) redraw = true; else Output();
				}
				else if (Thread::get_bool(mutex, _update) && isRawMode()) Output();
				else if (Thread::get_bool(mutex, _logo) && !isRawMode()) Logo();
				usleep(UPDATE_INTERVAL * 1000);
			}

			if (Thread::get_int(mutex, Settings::terminate) == 4) return (NULL);
			if (!background && !Settings::check_only && isRawMode() && !ForceRawModeDisabled) {
				usleep(100000); std::cout.flush(); std::cout.clear(); maxFails = 10; failCount = 0; drawing = false;
				Log::log(G + Settings::server_name + " " + Settings::server_version + " closed successfully", Log::GLOBAL_ACCESS); Log::log("---", Log::GLOBAL_ACCESS); Log::process_logs(); Output();
			} else if (!background && !Settings::check_only)
				std::cout << G  + Settings::server_name + " " + Settings::server_version + " closed successfully\n" << CSHOW << std::endl;
			else if (!background)
				std::cout << CSHOW << std::endl;
			return (NULL);
		}

	#pragma endregion

	#pragma region Start

		void Display::start() {
			_terminate = false;
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
