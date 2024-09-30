/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Files.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 22:23:52 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/01 00:28:57 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

#include <cstdio>																						//	For snprintf to get the modification time of a file
#include <fstream>

#include <errno.h>																						//	For errno to check if a directory exists in 'createPath'
#include <fcntl.h>																						//	For fcntl() to set an FD as non-blocking
#include <unistd.h>																						//	For readlink() and access() to read symbolic links and check file accessibility
#include <limits.h>																						//	For PATH_MAX (define the maximum path length)
#include <sys/stat.h>																					//	For mkdir() to get the size of a file
#include <pwd.h>																						//	For getpwuid() to retrieve the user home directory

#pragma region Program Path

	std::string Utils::programPath() {
		char result[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
		if (count != -1) {
			result[count] = '\0';
			std::string fullPath(result);

			std::string::size_type pos = fullPath.find_last_of('/');
			if (pos != std::string::npos) return (fullPath.substr(0, pos + 1));
		}

		char * home = getenv("HOME");
    	if (home) return (std::string(home) + "/");

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

	int Utils::file_exists(const std::string & path, bool is_exec) {
		struct stat path_stat;

		if (stat(path.c_str(), &path_stat) != 0)		return (1);										//	Check if the file exists
		if (!S_ISREG(path_stat.st_mode))				return (3);										//	Is not a file
		if (!is_exec && !(path_stat.st_mode & S_IRUSR))	return (2);										//	Check if the file has read permissions
		if (is_exec && !(path_stat.st_mode & S_IXUSR))	return (2);										//	Check if the file has execution permissions

		return (0);
	}

#pragma endregion

#pragma region Directory Exists

	int Utils::directory_exists(const std::string & path) {
		struct stat path_stat;

		if (stat(path.c_str(), &path_stat) != 0)		return (1);										//	Check if the file exists
		if (!S_ISDIR(path_stat.st_mode))				return (3);										//	Is not a directory
		if (!(path_stat.st_mode & S_IRUSR))	return (2);													//	Check if the file has read permissions

		return (0);
	}

#pragma endregion

#pragma region IsFile

	bool Utils::isFile(const std::string & path) {
		struct stat path_stat;

		if (stat(path.c_str(), &path_stat) != 0) return (false);
		return (S_ISREG(path_stat.st_mode));
	}

#pragma endregion

#pragma region IsDirectory

	bool Utils::isDirectory(const std::string & path) {
		struct stat path_stat;

		if (stat(path.c_str(), &path_stat) != 0) return (false);
		return (S_ISDIR(path_stat.st_mode));
	}

#pragma endregion

#pragma region FullPath

	std::string Utils::fullpath(const std::string & path) {
		std::vector<std::string> split_path; std::string dir;
		std::istringstream stream(path);

		while (std::getline(stream, dir, '/')) split_path.push_back(dir);

		std::vector<std::string> full_path;

		for (size_t i = 0; i < split_path.size(); ++i) {
			if (split_path[i] == "." || split_path[i].empty())	continue;
			else if (split_path[i] == "..") {
				if (!full_path.empty())							full_path.pop_back();
			} else												full_path.push_back(split_path[i]);
		}

		std::string real_path = "/";
		for (size_t i = 0; i < full_path.size(); ++i) {
			if (i > 0) real_path += "/";
			real_path += full_path[i];
		}

		return (real_path);
	}

#pragma endregion

#pragma region IsSubpath

	bool Utils::is_subpath(const std::string & path1, const std::string & path2) { return (path1.find(path2) == 0); }

#pragma endregion

#pragma region FileSize

	size_t Utils::filesize(const std::string & path) {
		if (path.empty()) return (std::string::npos);
		struct stat path_stat;
		
		if (stat(path.c_str(), &path_stat) != 0) return (std::string::npos);
		return (path_stat.st_size);
	}

	size_t Utils::filesize(const int fd) {
		if (fd == -1) return (std::string::npos);
		struct stat path_stat;

		if (fstat(fd, &path_stat) == -1) { return (std::string::npos); }
		return (path_stat.st_size);
	}

#pragma endregion

#pragma region Modification Time

	std::string Utils::file_modification_time(const std::string path, bool is_header) {
		struct stat fileInfo;
		if (stat(path.c_str(), &fileInfo) != 0) return ("");

		time_t modTime = fileInfo.st_mtime;

		char date[30];
		if (is_header) {																				//	Format for a HTTP header
			struct tm* timeInfo = gmtime(&modTime);
			strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);						//	Convert to GMT time
		} else {
			struct tm * timeInfo = localtime(&modTime);													//	Convert to local time
			strftime(date, sizeof(date), "%d/%m/%Y", timeInfo);
		}

		return (date);
	}

	time_t Utils::file_modification_time_data(const std::string & path) {
		struct stat fileInfo;

		if (stat(path.c_str(), &fileInfo) != 0) return ((time_t)(-1));
		
		return (fileInfo.st_mtime);
	}

#pragma endregion

#pragma region Expand Tilde

	std::string Utils::expand_tilde(const std::string & path) {
		const char * home = getenv("HOME");
		if (!home) home = getpwuid(getuid())->pw_dir;
		return (std::string(home) + path.substr(1));
	}

#pragma endregion

#pragma region Non-Blocking FD

	void Utils::NonBlocking_FD(int fd) {
		//return ;																							//	Disabled for now
		int flags = fcntl(fd, F_GETFL, 0);																	//	Get the current flags
		if (flags == -1) return;

		fcntl(fd, F_SETFL, flags | O_NONBLOCK);																//	Set the 'NONBLOCK' flag
	}

#pragma endregion

#pragma region OS Name

	std::string Utils::get_OSname() {
		std::ifstream osReleaseFile("/etc/os-release");
		if (!osReleaseFile.is_open()) return "";

		std::string line, osName;

		while (std::getline(osReleaseFile, line)) {
			if (line.find("PRETTY_NAME") != std::string::npos) {
				size_t pos = line.find('=');
				if (pos != std::string::npos) {
					osName = line.substr(pos + 2);
					if (!osName.empty()) osName = osName.substr(0, osName.size() - 1);
					break;
				}
			}
		}
		osReleaseFile.close();
		if (!osName.empty()) osName = " (" + osName + ")";
		return (osName);
	}

#pragma endregion
