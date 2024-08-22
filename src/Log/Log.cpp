/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Log.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/27 19:32:38 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/23 00:36:22 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"
#include "Display.hpp"
#include "Mutex.hpp"
#include "Log.hpp"

#pragma region Variables

	std::queue <Log::LogInfo>	Log::_logs;

	const size_t				Log::MEM_MAXSIZE = 200;														//	Maximum number of logs for each memory log
	const size_t				Log::LOCAL_MAXSIZE = 1 * 1024 * 1024;										//	Maximum size of the log in disk (default to 10 MB)

	#pragma region LogInfo

		#pragma region Constructors

			Log::LogInfo::LogInfo(std::string & _msg, int _type, VServer * _VServ, std::string _path) : msg(_msg), type(_type), VServ(_VServ), path(_path) {}

		#pragma endregion

		#pragma region Overloads

			Log::LogInfo &	Log::LogInfo::operator=(const LogInfo & rhs) {
				if (this != &rhs) { msg = rhs.msg; type = rhs.type; VServ = rhs.VServ; path = rhs.path; }
				return (*this);
			}

			bool			Log::LogInfo::operator==(const LogInfo & rhs) {
				return (msg == rhs.msg && type == rhs.type && VServ == rhs.VServ && path == rhs.path);
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Log

	#pragma region Constructors

		Log::Log() {}
		Log::Log(const Log & src) { *this = src; }
		Log::~Log() { clear(); }

	#pragma endregion

	#pragma region Overloads

		Log &	Log::operator=(const Log & rhs) { if (this != &rhs) { access = rhs.access; error = rhs.error; both = rhs.both; } return (*this); }
		bool	Log::operator==(const Log & rhs) const { return (access == rhs.access && error == rhs.error && both == rhs.both); }

	#pragma endregion

	#pragma region Memory Logs

		#pragma region Add

			void Log::access_add(const std::string & msg) {
				if (access.size() == MEM_MAXSIZE) access.pop_front();
					access.push_back(msg);
			}

			void Log::error_add(const std::string & msg) {
				if (error.size() == MEM_MAXSIZE) error.pop_front();
					error.push_back(msg);
			}

			void Log::both_add(const std::string & msg) {
				if (both.size() == MEM_MAXSIZE) both.pop_front();
					both.push_back(msg);
			}

		#pragma endregion

		#pragma region Clear

			void Log::clear() { access.clear(); error.clear(); both.clear(); }

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Static
	
	#pragma region Truncate Logs

		void Log::truncate_log(const std::string & path, long long maxFileSize) {
			const std::size_t blockSize = 8192;															//	Block size of data to read and write
			char buffer[blockSize];																		//	Buffer to read in blocks

			std::ifstream srcFile(path.c_str(), std::ios::binary);										//	Open the log file for reading
			if (!srcFile) return;

			srcFile.seekg(0, std::ios::end);															//	Go to the end of the file
			long long fileSize = srcFile.tellg();														//	Get the size of the file

			if (maxFileSize <= 0 || fileSize <= maxFileSize) return;									//	If the size of the file is lower than the maximum, do nothing

			srcFile.seekg(fileSize - maxFileSize, std::ios::beg);										//	Go to the begining position of the data tha we keep
			std::string line; std::getline(srcFile, line);												//	Go to the next line so we don't cut a line

			std::fstream dstFile(path.c_str(), std::ios::in | std::ios::out | std::ios::binary);		//	Open the log file for writing
			if (!dstFile) { srcFile.close(); return; }

			std::streampos writePos = 0;

			while (srcFile.read(buffer, blockSize) || srcFile.gcount() > 0) {							//	Read in blocks the data
				dstFile.seekp(writePos, std::ios::beg);													//	Go to the position for writing
				dstFile.write(buffer, srcFile.gcount());												//	Write the buffer in the file
				writePos += srcFile.gcount();															//	Update the position for the next write
			}
			dstFile.close();																			//	Close the file to save it to disk

			int fd = open(path.c_str(), O_WRONLY);														//	Open the log file for writing
			if (fd != -1) { ftruncate(fd, writePos); close(fd); }										//	Truncate the file from the las written position

			srcFile.close();
		}

	#pragma endregion

	#pragma region Log to Memory

		void Log::log_to_memory(std::string msg, int type, VServer * VServ) {
			if (msg.empty()) return ;
			if (Settings::check_only || Display::RawModeDisabled || Display::ForceRawModeDisabled) { std::cout << "\t" << msg << NC << std::endl; return; }

			if (type == MEM_ACCESS || type == BOTH_ACCESS) {
				Settings::global.log.both_add(msg); Settings::global.log.access_add(msg);
				if (VServ) { VServ->log.both_add(msg); VServ->log.access_add(msg); }
			}
			if (type == MEM_ERROR || type == BOTH_ERROR) {
				Settings::global.log.both_add(msg); Settings::global.log.error_add(msg);
				if (VServ) { VServ->log.both_add(msg); VServ->log.error_add(msg); }
			}
		}

	#pragma endregion

	#pragma region Log to File

		void Log::log_to_file(const std::string & msg, std::string path) {
			if (msg.empty() || path.empty()) return;
			truncate_log(path, (LOCAL_MAXSIZE) - msg.size());											//	Truncate the log file to the maximum set in the config file (default to 10 MB)
			std::ofstream outfile; outfile.open(path.c_str(), std::ios_base::app);
			if (outfile.is_open()) {
				outfile << msg; outfile.flush();
				outfile.close();
			}
		}

	#pragma endregion

	#pragma region Process Logs

		void Log::process_logs() {
			std::queue <Log::LogInfo> logs;

			Mutex::mtx_set(Mutex::MTX_LOCK);
			if (!_logs.empty()) std::swap(logs, _logs);
			Mutex::mtx_set(Mutex::MTX_UNLOCK);

			if (logs.empty()) return;

			std::map<std::string, std::string> logMap;

			while (!logs.empty()) {
				Log::LogInfo log = logs.front(); logs.pop();

				if (log.msg.empty()) continue;
				log.msg = "[" + Settings::timer.current_date() + " " + Settings::timer.current_time() + "] - " + log.msg;

				if (!log.path.empty() && log.path[0] != '/') log.path = Settings::program_path + log.path;

				if (log.type < 4) log_to_memory(log.msg, log.type, log.VServ);
				if (log.type > 1 && !log.path.empty()) logMap[log.path] += Utils::str_nocolor(log.msg) + "\n";
			}

			for (std::map<std::string, std::string>::iterator it = logMap.begin(); it != logMap.end(); ++it)
				log_to_file(it->second, it->first);
			logMap.clear();

			Display::Output();
		}

	#pragma endregion

	#pragma region Log

		void Log::log(std::string msg, int type, VServer * VServ, std::string path) {
			if (VServ == &(Settings::global)) VServ = NULL;

			if (path.empty()) {
				if (type == BOTH_ACCESS || type == LOCAL_ACCESS) {
					if (VServ) path = VServ->get("access_log");
					else path = Settings::global.get("access_log");
				}
				if (type == BOTH_ERROR || type == LOCAL_ERROR) {
					if (VServ) path = VServ->get("error_log");
					else path = Settings::global.get("error_log");
				}
			}

			Mutex::mtx_set(Mutex::MTX_LOCK);
			_logs.push(LogInfo(msg, type, VServ, path));
			Mutex::mtx_set(Mutex::MTX_UNLOCK);
		}
	
	#pragma endregion

#pragma endregion