/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:54:24 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/01 13:25:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Display.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Communication.hpp"

#pragma region Variables

	int			Epoll::epoll_fd = -1;																	//	File descriptor for epoll
	int			Epoll::timeout_fd = -1;																	//	File descriptor used to generating events in EPOLL and checking client timeouts

	const int	Epoll::MAX_EVENTS = 10;																	//	Maximum number of events that can be handled per iteration by epoll
	const int	Epoll::TIMEOUT_INTERVAL = 1;															//	Interval in seconds between timeout checks for inactive clients


#pragma endregion

#pragma region EPOLL

	#pragma region Time-Out

		#pragma region Create

			int Epoll::create_timeout() {
				timeout_fd = timerfd_create(CLOCK_MONOTONIC, 0);
				if (timeout_fd == -1) return (1);

				struct itimerspec new_value;
				memset(&new_value, 0, sizeof(new_value));
				new_value.it_value.tv_sec = TIMEOUT_INTERVAL;											//	Time to the first expiration
				new_value.it_interval.tv_sec = TIMEOUT_INTERVAL;										//	Interval between expirations

				if (timerfd_settime(timeout_fd, 0, &new_value, NULL) == -1) {
					Log::log(RD "Error creating " Y "Time-Out" RD " monitor" NC, Log::MEM_ERROR); Log::log("---", Log::MEM_ACCESS);
					::close(timeout_fd); timeout_fd = -1; return (1);
				}

				return (0);
			}

		#pragma endregion

		#pragma region Check

			void Epoll::clients_timeout() {
				uint64_t expirations;
				read(timeout_fd, &expirations, sizeof(expirations));

				long TimeOut = Settings::KEEP_ALIVE_TIMEOUT;

				if (Settings::global.get("keepalive_timeout") != "") Utils::stol(Settings::global.get("keepalive_timeout"), TimeOut);
				if (TimeOut == 0) return;

				std::list<Client>::iterator it = Communication::clients.begin();
				while (it != Communication::clients.end()) {
					std::list<Client>::iterator current = it; ++it;
					current->check_timeout(TimeOut);
				}
			}

			void Epoll::check_timeout() {
				Display::update();
				Event::check_timeout();
				clients_timeout();
				Communication::cache.remove_expired();
				Communication::cache.cleanup_caching();
			}

		#pragma endregion

	#pragma endregion

	#pragma region Create

		int Epoll::create() {
				if (epoll_fd != -1) close();

				epoll_fd = epoll_create(1024);
				if (epoll_fd == -1) {
					Log::log(RD "Fatal error creating " Y "EPOLL" NC, Log::MEM_ERROR); Log::log("---", Log::MEM_ACCESS);
					Settings::global.status = false; Settings::global.bad_config = true; return (1);
				}
				if (!create_timeout()) add(timeout_fd, true, false);

				return (0);
		}

	#pragma endregion

	#pragma region Close

		void Epoll::close() {
			if (timeout_fd != -1) { ::close(timeout_fd); timeout_fd = -1; }
			if (epoll_fd != -1) { ::close(epoll_fd); epoll_fd = -1; }
		}

	#pragma endregion
	
	#pragma region Add

		int Epoll::add(int fd, bool epollin, bool epollout) {
			if (fd < 0) return (-1);
			struct epoll_event epoll_event;

			epoll_event.data.fd = fd;

			if (epollin && epollout)	epoll_event.events = EPOLLIN | EPOLLOUT;
			else if (epollin) 			epoll_event.events = EPOLLIN;
			else if (epollout) 			epoll_event.events = EPOLLOUT;
			else { 						return (-1); }

			return (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_event));
		}

	#pragma endregion

	#pragma region Set

		int Epoll::set(int fd, bool epollin, bool epollout) {
			if (fd < 0) return (-1);
			struct epoll_event epoll_event;

			epoll_event.data.fd = fd;

			if (epollin && epollout)	epoll_event.events = EPOLLIN | EPOLLOUT;
			else if (epollin) 			epoll_event.events = EPOLLIN;
			else if (epollout)			epoll_event.events = EPOLLOUT;
			else {				 		return (0); }

			return (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &epoll_event));
		}

	#pragma endregion

	#pragma region Del

		void Epoll::remove(int fd) {
			if (epoll_fd < 0 || fd < 0) return;
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		}

	#pragma endregion

	#pragma region Events

		int Epoll::events() {																			//	This function is call from the loop in Main
			if (epoll_fd < 0) return (2);
			struct epoll_event events[MAX_EVENTS];

			int eventCount = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);
			if (eventCount == -1 && Display::signal == 0) {
				Log::log(RD "Fatal error in " Y "EPOLL" NC, Log::MEM_ERROR); Log::log("---", Log::MEM_ACCESS);
				Settings::global.status = false; Settings::global.bad_config = true; return (1);
			}

			for (int i = 0; i < eventCount; ++i) {
				if (events[i].data.fd == timeout_fd) { check_timeout(); continue; }

				EventInfo * event = Event::get(events[i].data.fd);
				if (!event) continue;

				if (events[i].events & EPOLLIN) {														//	Check for READ
					switch (event->type) {
						case SOCKET: 	{ Socket::accept(event);				break; }
						case CLIENT: 	{ Communication::read_client(event);	break; }
						case DATA: 		{ Communication::read_data(event);		break; }
						case CGI: 		{ Communication::read_cgi(event);		break; }
					}
				}

				event = Event::get(events[i].data.fd);
				if (!event) continue;

				if (events[i].events & EPOLLOUT) {														//	Check for WRITE
					switch (event->type) {
						case CLIENT: 	{ Communication::write_client(event);	break; }
						case DATA: 		{										break; }
						case CGI: 		{ Communication::write_cgi(event);		break; }
					}
				}
			}

			Socket::server_status();

			return (0);
		}

	#pragma endregion

#pragma endregion
