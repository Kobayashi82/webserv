/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:21:01 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/21 00:13:59 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"

#pragma region Variables

	std::map <int, EventInfo>			Event::events;												//	Map of all events objects

#pragma endregion

#pragma region EventInfo

	#pragma region Constructors

		EventInfo::EventInfo() : fd(-1), type(NOTHING), socket(NULL), client(NULL) {
			pipe[0] = -1; pipe[1] = -1; file_read = 0; file_size = 0; file_path = ""; no_cache = false; close = false; vserver_data = NULL; file_info = 0; cgi_fd = -1;
			body_size = 0; body_maxsize = 0; last_activity = std::time(NULL);
		}

		EventInfo::EventInfo(int _fd, int _type, SocketInfo * _socket, Client * _client) : fd(_fd), type(_type), socket(_socket), client(_client) {
			pipe[0] = -1; pipe[1] = -1; file_read = 0; file_size = 0; file_path = ""; no_cache = false; close = false; vserver_data = NULL; file_info = 0; cgi_fd = -1;
			body_size = 0; body_maxsize = 0; last_activity = std::time(NULL);
		}

		EventInfo::EventInfo(const EventInfo & src) { *this = src; }

	#pragma endregion

	#pragma region Overloads

		EventInfo & EventInfo::operator=(const EventInfo & rhs) {
			if (this != &rhs) {
				fd = rhs.fd; type = rhs.type; socket = rhs.socket; client = rhs.client; file_path = rhs.file_path; no_cache = rhs.no_cache; close = rhs.close; request = rhs.request; vserver_data = rhs.vserver_data;
				pipe[0] = rhs.pipe[0]; pipe[1] = rhs.pipe[1]; file_read = rhs.file_read; file_size = rhs.file_size; read_buffer = rhs.read_buffer; write_buffer = rhs.write_buffer; file_info = rhs.file_info;  cgi_fd = rhs.cgi_fd;
				header = rhs.header; header_map = rhs.header_map; method = rhs.method; body_size = rhs.body_size; body_maxsize = rhs.body_maxsize; last_activity = rhs.last_activity;
			}
			return (*this);
		}

		bool EventInfo::operator==(const EventInfo & rhs) const {
			return (fd == rhs.fd);
		}

	#pragma endregion

#pragma endregion

#pragma region Events

	#pragma region Get

		EventInfo * Event::get(int fd) {
			if (fd < 0) return (NULL);
			std::map<int, EventInfo>::iterator it = events.find(fd);

			if (it != events.end())		return (&it->second);
			else 						return (NULL);
		}

	#pragma endregion

	#pragma region Remove

		#pragma region Remove (All)

			void Event::remove() {
				if (events.size() == 0) return;
				std::map<int, EventInfo>::iterator it = events.begin();
				while (it != events.end()) {
					std::map<int, EventInfo>::iterator current = it++;
					Epoll::remove(current->second.fd);
					if (current->second.fd != -1) close(current->second.fd);
					if (current->second.type == DATA) {
						if (current->second.pipe[0] != -1) close(current->second.pipe[0]);
						if (current->second.pipe[1] != -1) close(current->second.pipe[1]);
					}
					events.erase(current);
				}
			}

		#pragma endregion

		#pragma region Remove (Client)

			void Event::remove(Client * Cli) {
				if (events.size() == 0) return;
				std::map<int, EventInfo>::iterator it = events.begin();
				while (it != events.end()) {
					std::map<int, EventInfo>::iterator current = it++;
					if (current->second.client == Cli) {
						Epoll::remove(current->second.fd);
						if (current->second.fd != -1) close(current->second.fd);
						if (current->second.type == DATA) {
							if (current->second.pipe[0] != -1) close(current->second.pipe[0]);
							if (current->second.pipe[1] != -1) close(current->second.pipe[1]);
						}
						events.erase(current);
					}
				}
			}

		#pragma endregion

		#pragma region Remove (One)

			int Event::remove(int fd) {
				EventInfo * event = get(fd);
				if (!event) return (1);

				Epoll::remove(event->fd);
				if (event->fd != -1) close(event->fd);
				if (event->type == DATA) {
					if (event->pipe[0] != -1) close(event->pipe[0]);
					if (event->pipe[1] != -1) close(event->pipe[1]);
				}
				events.erase(event->fd);

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region Time-Out

		void Event::check_timeout(int interval) {
			if (events.size() == 0) return;

			time_t current_time = std::time(NULL);

			std::map<int, EventInfo>::iterator it = events.begin();
			while (it != events.end()) {
				if (it->second.type == SOCKET) { it++; continue; }
				if (difftime(current_time, it->second.last_activity) > interval) {
					if (it->second.type == CLIENT)	{ it->second.client->remove(); it = events.begin(); }
					else							{
						std::map<int, EventInfo>::iterator current = it++;
						remove(current->first);
					}
				} else it++;
			}
		}

		void Event::update_last_activity(int fd) {
			EventInfo * event = get(fd);
			if (!event) return;

			event->last_activity = std::time(NULL);
		}

	#pragma endregion

#pragma endregion