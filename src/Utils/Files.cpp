/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Files.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 22:23:52 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/10 22:51:11 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

#include <errno.h>																						//	For errno
#include <unistd.h>																						//	For readlink() and access()
#include <limits.h>																						//	For PATH_MAX
#include <sys/stat.h>																					//	For mkdir()

#pragma region Program Path

	std::string Utils::programPath() {
		char result[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
		if (count != -1) return (std::string(result, count - 7));
		return ("/");
	}

#pragma endregion

#pragma region Create Path

	int Utils::createPath(const std::string & path) {
		size_t pos = 0; std::string dir;

		if (path == "") return (0);
		while ((pos = path.find('/', pos)) != std::string::npos) {
			dir = path.substr(0, pos++);
			if (dir.empty()) continue;
			if (mkdir(dir.c_str(), 0755) == -1 && errno != EEXIST) return (1);
		} return (0);
	}

#pragma endregion

#pragma region File Exists

	int Utils::file_exists(const std::string & File) {
		if (access(File.c_str(), F_OK) < 0) return (1);
		if (access(File.c_str(), R_OK) < 0) return (2);
		return (0);
	}

#pragma endregion

#pragma region IsFile

	bool Utils::isFile(const std::string & path) {
		struct stat path_stat; stat(path.c_str(), &path_stat);
		return (S_ISREG(path_stat.st_mode));
	}

#pragma endregion

#pragma region IsDirectory

	bool Utils::isDirectory(const std::string & path) {
		struct stat path_stat; stat(path.c_str(), &path_stat);
		return (S_ISDIR(path_stat.st_mode));
	}

#pragma endregion
