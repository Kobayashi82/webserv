/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Event.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 11:21:01 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/07 14:33:13 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Protocol.hpp"
#include "Communication.hpp"

#pragma region Variables

	std::map <int, EventInfo>			Event::events;												//	Map of all events objects

#pragma endregion

#pragma region EventInfo

	#pragma region Constructors

		EventInfo::EventInfo() : fd(-1), type(NOTHING), socket(NULL), client(NULL) {
			pipe[0] = -1; pipe[1] = -1;
			read_path = ""; read_size = 0; read_maxsize = 0; read_info = 0;
			write_path = ""; write_size = 0; write_maxsize = 0; write_info = 0; cgi_fd = -1; cgi_read_fd = -1;
			
			no_cache = false; close = false;
			VServ = NULL; Loc = NULL; vserver_data = NULL;

			response_size = 0; body_size = 0; body_maxsize = 0; redirect_status = 0;
			last_activity = std::time(NULL); gettimeofday(&response_time, NULL);
		}

		EventInfo::EventInfo(int _fd, int _type, SocketInfo * _socket, Client * _client) : fd(_fd), type(_type), socket(_socket), client(_client) {
			pipe[0] = -1; pipe[1] = -1;
			read_path = ""; read_size = 0; read_maxsize = 0; read_info = 0;
			write_path = ""; write_size = 0; write_maxsize = 0; write_info = 0; cgi_fd = -1; cgi_read_fd = -1;
			
			no_cache = false; close = false; mod_time = time(NULL); filesize = 0;
			VServ = NULL; Loc = NULL; vserver_data = NULL;

			response_size = 0; body_size = 0; body_maxsize = 0; redirect_status = 0;
			last_activity = std::time(NULL); gettimeofday(&response_time, NULL); pid = 0;
		}

		EventInfo::EventInfo(const EventInfo & src) { *this = src; }

	#pragma endregion

	#pragma region Overloads

		EventInfo & EventInfo::operator=(const EventInfo & rhs) {
			if (this != &rhs) {
				fd = rhs.fd; type = rhs.type; socket = rhs.socket; client = rhs.client;
				
				pipe[0] = rhs.pipe[0]; pipe[1] = rhs.pipe[1];
				read_path = rhs.read_path; read_size = rhs.read_size; read_maxsize = rhs.read_maxsize; read_info = rhs.read_info;
				write_path = rhs.write_path; write_size = rhs.write_size; write_maxsize = rhs.write_maxsize; write_info = rhs.write_info; cgi_fd = rhs.cgi_fd; cgi_read_fd = rhs.cgi_read_fd;

				no_cache = rhs.no_cache; close = rhs.close; mod_time = rhs.mod_time; filesize = rhs.filesize;
				VServ = rhs.VServ; Loc = rhs.Loc; vserver_data = rhs.vserver_data;

				read_buffer = rhs.read_buffer; write_buffer = rhs.write_buffer;

				header = rhs.header; header_map = rhs.header_map;
				response_map = rhs.response_map; response_size = rhs.response_size; response_method = rhs.response_method;				
				body_size = rhs.body_size; body_maxsize = rhs.body_maxsize; redirect_status = rhs.redirect_status;

				response_time = rhs.response_time; last_activity = rhs.last_activity; pid = rhs.pid;
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
					if (current->second.type == DATA || current->second.type == CGI) {
						if (current->second.pipe[0] != -1) close(current->second.pipe[0]);
						if (current->second.pipe[1] != -1) close(current->second.pipe[1]);
					}
					events.erase(current);
				}
			}

		#pragma endregion

		#pragma region Remove (Client)

			void Event::remove(Client * Cli) {
				std::map<int, EventInfo>::iterator it = events.begin();
				while (it != events.end()) {
					std::map<int, EventInfo>::iterator current = it++;
					if (current->second.client == Cli) {
						Epoll::remove(current->second.fd);
						if (current->second.fd != -1) close(current->second.fd);
						if (current->second.type == DATA || current->second.type == CGI) {
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
				if (event->type == DATA || event->type == CGI) {
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
				if (it->second.type == SOCKET ||it->second.type == CLIENT) { it++; continue; }
				if (difftime(current_time, it->second.last_activity) > interval) {
					std::map<int, EventInfo>::iterator current = it++;
					EventInfo * event = get(current->first);
					if (!event) continue;
					EventInfo * c_event = get(event->client->fd);
					if (!c_event) continue;
					remove(current->first);
					if (c_event->pid != 0) { kill(c_event->pid, SIGKILL); c_event->pid = 0; }
					c_event->header_map["Connection"] = "close";
					c_event->response_map["Connection"] = "close";
					Protocol::check_code(c_event, true, "408");
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
