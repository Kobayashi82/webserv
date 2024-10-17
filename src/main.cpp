/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/05 14:15:33 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Log.hpp"
#include "Display.hpp"
#include "Thread.hpp"
#include "Settings.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Communication.hpp"

#pragma region Reload

	void reload(std::string current_path, int argc, char **argv) {
		Epoll::close();
		Socket::close();
		Event::remove();

		Log::stop(); Display::stop(); Log::release_mutex(); Display::disableRawMode();

		Communication::total_clients = 0;
		Communication::write_bytes = 0;
		Communication::read_bytes = 0;
		Communication::cache.clear();

		Settings::clear(true);

		chdir(current_path.c_str());

		Settings::load_args(argc, argv);

		Display::signal_handler();
		Log::start(); Display::start();
				
		Epoll::create();
		Socket::create(true);

		usleep(10000); Display::update();
	}

#pragma endregion

#pragma region Main

	int main(int argc, char **argv) {
		Settings::load_args(argc, argv);

		std::string current_path;
		char get_path[PATH_MAX];
		if (getcwd(get_path, sizeof(get_path)) != NULL) current_path = get_path;
		current_path = Utils::fullpath(current_path);

		Display::signal_handler();
		Log::start(); Display::start();
		
		Epoll::create();
		Socket::create(true);

		usleep(10000); Display::update();

		while (Display::isTerminate() == -1) {
			if (Display::signal_check())	break;
			if (Epoll::events())			break;
			if (Display::isTerminate() == 4) reload(current_path, argc, argv);
		}
		
		for (std::deque<VServer>::iterator it = Settings::vserver.begin(); it != Settings::vserver.end(); ++it)
			Thread::set_bool(Display::mutex, it->force_off, true);
		Thread::set_bool(Display::mutex, Settings::global.status, false);

		Epoll::close();
		Socket::close();
		Event::remove();

		Log::stop(); Display::stop(); Log::release_mutex(); Display::disableRawMode();

		Log::exec_logrot(Utils::fullpath(Settings::program_path + "/.logrotate.cfg"));
		
		return (Settings::terminate);
	}

#pragma endregion
