/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 11:59:50 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/07 13:58:36 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Protocol.hpp"
#include "Communication.hpp"

#pragma region Responses

	#pragma region Error

		#pragma region Error Body

			static void error_body(std::string & body, std::string & code, std::string & description) {
				body +=
					"			<!DOCTYPE html>\n"
					"<html lang=\"es\">\n"
					"<head>\n"
					"    <meta charset=\"UTF-8\">\n"
					"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
					"    <title>Error " + code + " - " + description + "</title>\n"
					"    <style>\n"
					"        body {\n"
					"            margin: 0;\n"
					"            font-family: 'Arial', sans-serif;\n"
					"            background: linear-gradient(135deg, #f06, #ffcc33);\n"
					"            display: flex;\n"
					"            justify-content: center;\n"
					"            align-items: center;\n"
					"            height: 100vh;\n"
					"            color: #fff;\n"
					"        }\n"
					"        .error-container {\n"
					"            text-align: center;\n"
					"            background-color: rgba(0, 0, 0, 0.6);\n"
					"            padding: 40px;\n"
					"            border-radius: 15px;\n"
					"            box-shadow: 0 15px 25px rgba(0, 0, 0, 0.5);\n"
					"        }\n"
					"        .error-code {\n"
					"            font-size: 6rem;\n"
					"            font-weight: bold;\n"
					"            margin: 0;\n"
					"        }\n"
					"        .error-message {\n"
					"            font-size: 1.5rem;\n"
					"            margin-top: 10px;\n"
					"            color: #ddd;\n"
					"        }\n"
					"        .back-link {\n"
					"            margin-top: 20px;\n"
					"            display: inline-block;\n"
					"            padding: 10px 20px;\n"
					"            background-color: #fff;\n"
					"            color: #333;\n"
					"            border-radius: 25px;\n"
					"            text-decoration: none;\n"
					"            font-weight: bold;\n"
					"            transition: background-color 0.3s ease;\n"
					"        }\n"
					"        .back-link:hover {\n"
					"            background-color: #f06;\n"
					"            color: #fff;\n"
					"        }\n"
					"    </style>\n"
					"</head>\n"
					"<body>\n"
					"    <div class=\"error-container\">\n"
					"        <div class=\"error-code\">" + code + "</div>\n"
					"        <div class=\"error-message\">" + description + "</div>\n"
					"        <a href=\"/\" class=\"back-link\">Volver al inicio</a>\n"
					"    </div>\n"
					"</body>\n"
					"</html>";
			}

		#pragma endregion

		#pragma region Error

			void Protocol::response_error(EventInfo * event) {
				if (!event) return;

				std::string code = event->response_map["Code"];
				std::string description = event->response_map["Code-Description"];

				std::string body;

				error_body(body, code, description);

				std::string header =
					event->response_map["Protocol"] + " " + code + " " + description + "\r\n"
					"Server: " + event->response_map["Server"] + "\r\n"
					"Date: " + event->response_map["Date"] + "\r\n"
					"Cache-Status: MISS" + "\r\n"
					"Cache-Control: no-store, no-cache" + "\r\n"
					"Content-Type: " + Settings::mime_types["html"] + "\r\n"
					"X-Content-Type-Options: nosniff\r\n"
					"Content-Length: " + Utils::ltos(body.size()) + "\r\n"
					"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

				event->write_info = 0;																	//	Set some flags
				event->write_size = 0;																	//	Set some flags
				event->write_maxsize = 0;																//	Set some flags
				event->response_map["Header"] = header;
				event->write_buffer.clear();															//	Clear write_buffer
				event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());	//	Copy the header to write_buffer

				if (event->header_map["Method"] != "HEAD") {
					event->response_map["Body"] = body;
					event->response_size = body.size();
					event->write_maxsize = header.size() + body.size();										//	Set the total size of the data to be sent
					event->write_buffer.insert(event->write_buffer.end(), body.begin(), body.end());		//	Copy the body to write_buffer
				}

				if (Epoll::set(event->fd, true, true) == -1) event->client->remove();					//	Set EPOLL to monitor write events
			}

		#pragma endregion

	#pragma endregion

	#pragma region Redirect

		void Protocol::response_redirect(EventInfo * event) {
			if (!event) return;

			std::string header =
				event->response_map["Protocol"] + " " + event->response_map["Code"] + " " + event->response_map["Code-Description"] + "\r\n"
				"Server: " + event->response_map["Server"] + "\r\n"
				"Date: " + event->response_map["Date"] + "\r\n"
				"Cache-Status: MISS" + "\r\n"
				"Cache-Control: no-store, no-cache" + "\r\n"
				"Location: " + Security::encode_url(event->response_map["Path"]) + "\r\n"
				"X-Content-Type-Options: nosniff" + "\r\n"
				"Content-Length: 0" + "\r\n"
				"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

			event->response_map["Header"] = header;
			event->write_info = 0;																	//	Set some variables
			event->write_size = 0;																	//	Set some variables
			event->response_size = 0;																//	Set some variables
			event->write_maxsize = header.size();													//	Set the total size of the data to be sent
			event->write_buffer.clear();															//	Clear write_buffer
			event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());	//	Copy the header to write_buffer
			if (Epoll::set(event->fd, true, true) == -1) event->client->remove();					//	Set EPOLL to monitor write events
		}

	#pragma endregion

	#pragma region Directory

		#pragma region Add Style

			static void add_style(EventInfo * event, std::string & body, std::string dir_path, const std::string root) {
				if (Protocol::isAlias) dir_path = event->response_map["Old-Path"];
				if (dir_path == ".") dir_path = "";
				if (dir_path[0] != '/') dir_path = "/" + dir_path;
				if (dir_path[dir_path.size() - 1] != '/') dir_path += "/";
				body +=
					"<!DOCTYPE html>\n"
					"<html lang=\"en\">\n"
					"<head>\n"
					"    <meta charset=\"UTF-8\">\n"
					"    <title>Index of " + dir_path + "</title>\n"
					"    <style>\n"
					"        body {\n"
					"            font-family: Arial, sans-serif;\n"
					"            background-color: #f4f4f4;\n"
					"            margin: 0;\n"
					"            padding: 20px;\n"
					"        }\n"
					"        h1 {\n"
					"            color: #333;\n"
					"        }\n"
					"        table {\n"
					"            width: 100%;\n"
					"            border-collapse: collapse;\n"
					"            margin-top: 20px;\n"
					"        }\n"
					"        th, td {\n"
					"            padding: 10px;\n"
					"            text-align: left;\n"
					"            border-bottom: 1px solid #ccc;\n"
					"        }\n"
					"        th {\n"
					"            background-color: #4CAF50;\n"
					"            color: white;\n"
					"        }\n"
					"        .directory {\n"
					"            font-weight: bold;\n"
					"            color: #007BFF;\n"
					"        }\n"
					"        .file {\n"
					"            color: #333;\n"
					"        }\n"
					"        .size {\n"
					"            text-align: right;\n"
					"        }\n"
					"        .date {\n"
					"            text-align: center;\n"
					"        }\n"
					"    </style>\n"
					"</head>\n"
					"<body>\n"
					"    <h1>Index of " + dir_path + "</h1>\n"
					"    <table>\n"
					"        <thead>\n"
					"            <tr>\n"
					"                <th>Name</th>\n"
					"                <th class=\"size\">Size</th>\n"
					"                <th class=\"date\">Last Modified</th>\n"
					"            </tr>\n"
					"        </thead>\n"
					"        <tbody>\n";

				if (dir_path != "/" && dir_path != root) body +=
					"            <tr>\n"
					"                <td><strong><a class=\"directory\" href=\"../\">Parent Directory</a></strong></td>\n"
					"                <td></td>\n"
					"                <td></td>\n"
					"            </tr>\n";
			}

		#pragma endregion

		#pragma region Add Directory

			static void add_dir(EventInfo * event, std::string & body, std::string dir_path, const std::string & dir) {
				if (Protocol::isAlias) dir_path = event->response_map["Old-Path"];
				if (dir_path == ".") dir_path = "";
				dir_path = "/" + dir_path;
				if (dir_path[dir_path.size() - 1] != '/') dir_path += "/";
				body +=
				"<tr>\n"
				"    <td><strong><a class=\"directory\" href=" + Security::encode_url(dir_path + dir) + ">" + dir + "</a></strong></td>\n"
				"    <td></td>\n"
				"    <td></td>\n"
				"</tr>\n";
			}

		#pragma endregion

		#pragma region Add File

			static void add_file(EventInfo * event, std::string & body, std::string dir_path, const std::string & file) {
				if (Protocol::isAlias) dir_path = event->response_map["Old-Path"];
				if (dir_path == ".") dir_path = "";
				dir_path = "/" + dir_path;
				if (dir_path[dir_path.size() - 1] != '/') dir_path += "/";
				size_t filesize = Utils::filesize(file);
				std::string mod_time = Utils::file_modification_time(file);
				body +=
				"<tr>\n"
					"<td><a class=\"file\" href=" + Security::encode_url(dir_path + file) + ">" + file + "</a></td>\n"
					"<td class=\"size\">" + Utils::formatSize(filesize) + "</td>\n"
					"<td class=\"date\">" + mod_time + "</td>\n"
				"</tr>\n";
			}

		#pragma endregion

		#pragma region Directory

			void Protocol::response_directory(EventInfo * event) {
				std::vector<std::string> files, directories;
				
				std::string dir_path = event->response_map["Path-Full"];
				if (dir_path == "/") dir_path = ".";

				DIR *dir = opendir(".");																			//	Open the directory
				if (!dir) {																							//	If error, return error... duh
					event->response_map["Method"] ="Error";
					event->response_map["Code"] = "404";
					event->response_map["Code-Description"] = Settings::error_codes[Utils::sstol(event->response_map["Code"])];
					response_error(event);
					return;
				}

				struct dirent *entry;
				while ((entry = readdir(dir)) != NULL) {															//	Read the content of the directory
					std::string name = entry->d_name;

					if (name == "." || name == "..") continue;														//	ignore . and ..

					if (Utils::isDirectory(name))	directories.push_back(name + "/");								//	Add the directory to a list of directories
					else							files.push_back(name);											//	Add the file to a list of files
				}

				closedir(dir);																						//	Close the directory

				std::string body;																					//	Create the body of the response
				add_style(event, body, dir_path, event->response_map["Root"]);												//	Add the column and style to the body

				for (std::vector<std::string>::iterator it = directories.begin(); it != directories.end(); ++it)	//	Add directories to the body
					add_dir(event, body, dir_path, *it);

				for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)				//	Add files to the body
					add_file(event, body, dir_path, *it);

				body +=																								//	Finish the body of the response
					"       </tbody>\n"
					"    </table>\n"
					"</body>\n"
					"</html>";

				std::string header =																				//	Create the header of the response
					event->response_map["Protocol"] + " " + event->response_map["Code"] + " " + event->response_map["Code-Description"] + "\r\n"
					"Server: " + event->response_map["Server"] + "\r\n"
					"Date: " + event->response_map["Date"] + "\r\n"
					"Cache-Status: MISS" + "\r\n"
					"Cache-Control: no-store, no-cache" + "\r\n"
					"Content-Type: " + Settings::mime_types["html"] + "\r\n"
					"X-Content-Type-Options: nosniff" + "\r\n"
					"Content-Length: " + Utils::ltos(body.size()) + "\r\n"
					"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

				event->response_map["Header"] = header;
				event->response_map["Body"] = body;
				event->write_info = 0;																				//	Set some flags
				event->write_size = 0;																				//	Set some flags
				event->write_maxsize = 0;																			//	Set some flags
				event->write_buffer.clear();																		//	Clear write_buffer
				event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());				//	Copy the header to write_buffer
		
				if (event->header_map["Method"] != "HEAD") {														//	If method is not HEAD
					event->response_size = body.size();																//	Set the size of the response
					event->write_maxsize = header.size() + body.size();												//	Set the total size of the data to be sent
					event->write_buffer.insert(event->write_buffer.end(), body.begin(), body.end());				//	Copy the body to write_buffer
				}

				if (Epoll::set(event->fd, true, true) == -1) event->client->remove();								//	Set EPOLL to monitor write events

			}

		#pragma endregion

	#pragma endregion

	#pragma region File

		#pragma region Cache

			int Protocol::file_cache(EventInfo * event, std::string & path) {
				if (!event->no_cache || !Communication::cache.get_caching(path)) {													//	If cache is allowed
					try {
						CacheInfo & fcache = Communication::cache.get(path);														//	Get the file from cache

						std::string header; std::string content_length = "0";
						size_t filesize = fcache.content.size(), start = 0, end = filesize;

						if (event->response_map["Range"].size() > 7) {																//	Send a fragment of the file
							size_t temp = 0;
							std::string value = event->response_map["Range"].substr(6);
							if (value[0] == '-') {																					//	bytes=-1024		The client asks for the last 1024 bytes of the file
								if (Utils::stol(value.substr(1), temp) || filesize > temp) { return (0); } // error
								start = filesize - temp;
							} else if (value[value.size() - 1] == '-') {															//	bytes=1024-		The client asks for all bytes starting from byte 1024 to the end of the file
								if (Utils::stol(value.substr(0, value.size() - 1), temp) || filesize > temp) { return (0); } // error
								start = temp;
							} else {																								//	bytes=0-1023	The client asks for bytes from 0 to 1023 (inclusive)
								size_t pos = value.find_first_of('-');
								if (pos == std::string::npos) { return (0); } // error
								if (Utils::stol(value.substr(0, pos), temp) || filesize > temp) { return (0); } // error
								start = temp;
								if (Utils::stol(value.substr(pos + 1), temp) || filesize > temp) { return (0); } // error
								end = temp;
							}
							header =
								event->response_map["Protocol"] + " " + event->response_map["Code"] + " " + event->response_map["Code-Description"] + "\r\n"
								"Server: " + event->response_map["Server"] + "\r\n"
								"Date: " + event->response_map["Date"] + "\r\n"
								"Cache-Status: HIT" + "\r\n"
								"Age: " + Utils::str_time(time(NULL) - fcache.added_time) + "\r\n"
								"Last-Modified: " + fcache.mod_stime + "\r\n"
								"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
								"Cache-Control: public, max-age=3600" + "\r\n"
								"X-Content-Type-Options: nosniff" + "\r\n"
								"Content-Range: bytes " + Utils::ltos(start) + "-" + Utils::ltos(end) + "/" + Utils::ltos(filesize) + "\r\n"
								"Content-Length: " + Utils::ltos(end - start) + "\r\n"
								"Connection: " + event->response_map["Connection"] + "\r\n\r\n";
						} else {
							header =
								event->response_map["Protocol"] + " " + event->response_map["Code"] + " " + event->response_map["Code-Description"] + "\r\n"
								"Server: " + event->response_map["Server"] + "\r\n"
								"Date: " + event->response_map["Date"] + "\r\n"
								"Cache-Status: HIT" + "\r\n"
								"Age: " + Utils::str_time(time(NULL) - fcache.added_time) + "\r\n"
								"Last-Modified: " + fcache.mod_stime + "\r\n"
								"Cache-Control: no-store, no-cache" + "\r\n"
								"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
								"X-Content-Type-Options: nosniff" + "\r\n"
								"Content-Length: " + Utils::ltos(end - start) + "\r\n"
								"Connection: " + event->response_map["Connection"] + "\r\n\r\n";
						}

						event->write_info = 0;																						//	Set some flags
						event->write_size = 0;																						//	Set some flags
						event->write_maxsize = header.size();
						event->response_size = 0;
						event->write_buffer.clear();																				//	Clear write_buffer
						event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());						//	Copy the header to write_buffer

						if (event->header_map["Method"] != "HEAD") {
							event->write_maxsize = header.size() + (end - start);													//	Set some flags
							event->response_size = end - start;
							event->write_buffer.insert(event->write_buffer.end(), fcache.content.begin(), fcache.content.end());	//	Copy the body to write_buffer
						}

						if (Epoll::set(event->fd, true, true) == -1) event->client->remove();										//	Set EPOLL to monitor write events

						return (1);

					} catch(...) { return (0); }
				}
				return (0);
			}

		#pragma endregion

		#pragma region File

			void Protocol::response_file(EventInfo * event) {
				if (!event) return;

				std::string path = event->response_map["Path-Full"];
				if (path.empty()) { event->client->remove(); return; }

				if (file_cache(event, path)) return;
				Communication::cache.add_caching(path);
				int fd = open(path.c_str(), O_RDONLY);																	//	Open the file
				if (fd == -1) return;																					//	If error opening, return
				Utils::NonBlocking_FD(fd);																				//	Set the FD as non-blocking

				size_t filesize = event->filesize;																		//	Get the file size
				if (filesize == std::string::npos) { close(fd); event->client->remove(); return; }

				std::string header; std::string content_length = "0";
				size_t start = 0, end = filesize;

				if (event->response_map["Range"].size() > 7) {															//	Send a fragment of the file
					size_t temp = 0;
					std::string value = event->response_map["Range"].substr(6);
					if (value[0] == '-') {																				//	bytes=-1024		The client asks for the last 1024 bytes of the file
						if (Utils::stol(value.substr(1), temp) || filesize > temp) { return; }							// error
						start = filesize - temp;
					} else if (value[value.size() - 1] == '-') {														//	bytes=1024-		The client asks for all bytes starting from byte 1024 to the end of the file
						if (Utils::stol(value.substr(0, value.size() - 1), temp) || filesize > temp) { return; }		// error
						start = temp;
					} else {																							//	bytes=0-1023	The client asks for bytes from 0 to 1023 (inclusive)
						size_t pos = value.find_first_of('-');
						if (pos == std::string::npos) { return; }														// error
						if (Utils::stol(value.substr(0, pos), temp) || filesize > temp) { return; }						// error
						start = temp;
						if (Utils::stol(value.substr(pos + 1), temp) || filesize > temp) { return; }					// error
						end = temp;
					}
					header =
						event->response_map["Protocol"] + " " + "206" + " " + "Partial Content" + "\r\n"
						"Server: " + event->response_map["Server"] + "\r\n"
						"Date: " + event->response_map["Date"] + "\r\n"
						"Cache-Status: MISS" + "\r\n"
						"Last-Modified: " + event->response_map["Last-Modified"] + "\r\n"
						"Accept-Ranges: bytes" + "\r\n"
						"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
						"Cache-Control: public, max-age=3600" + "\r\n"
						"X-Content-Type-Options: nosniff" + "\r\n"
						"Content-Range: bytes " + Utils::ltos(start) + "-" + Utils::ltos(end) + "/" + Utils::ltos(filesize) + "\r\n"
						"Content-Length: " + Utils::ltos(end - start) + "\r\n"
						"Connection: " + event->response_map["Connection"] + "\r\n\r\n";
				} else {
					header =
						event->response_map["Protocol"] + " " + event->response_map["Code"] + " " + event->response_map["Code-Description"] + "\r\n"
						"Server: " + event->response_map["Server"] + "\r\n"
						"Date: " + event->response_map["Date"] + "\r\n"
						"Cache-Status: MISS" + "\r\n"
						"Last-Modified: " + event->response_map["Last-Modified"] + "\r\n"
						"Accept-Ranges: bytes" + "\r\n"
						"Cache-Control: no-store, no-cache" + "\r\n"
						"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
						"X-Content-Type-Options: nosniff" + "\r\n"
						"Content-Length: " + Utils::ltos(end - start) + "\r\n"
						"Connection: " + event->response_map["Connection"] + "\r\n\r\n";
				}

				if (event->header_map["Method"] == "HEAD") {
					close(fd);

					event->write_info = 0;																				//	Set some flags
					event->write_size = 0;																				//	Set some flags
					event->write_maxsize = header.size();																//	Set some flags
					event->write_buffer.clear();																		//	Clear write_buffer
					event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());				//	Copy the header to write_buffer

					if (Epoll::set(event->fd, !(event->header_map["Write-Only"] == "true"), true) == -1)				//	Set EPOLL to monitor write events
						event->client->remove();

					return;
				}

				if (start != 0 && lseek(fd, start, SEEK_SET) == -1) { close(fd); return; }								//	Move the pointer in the file to the start position

				EventInfo event_data(fd, DATA, NULL, event->client);													//	Create the event for the DATA

				event_data.read_path = path;																			//	Set the name of the file
				event_data.read_maxsize = end - start;																	//	Set the size of the file

				if (filesize > Communication::cache.max_content_size) event->no_cache = true;							//	No cache if file is greater than maximum allowed
				event_data.no_cache = event->no_cache;																	//	Set if the file must be added to cache

				if (pipe(event_data.pipe) == -1) { close(event_data.fd); return; event->client->remove(); }				//	Create the pipe for DATA (to be used with splice)
				Utils::NonBlocking_FD(event_data.pipe[0]);																//	Set the read end of the pipe as non-blocking
				Utils::NonBlocking_FD(event_data.pipe[1]);																//	Set the write end of the pipe as non-blocking

				size_t chunk = Communication::CHUNK_SIZE;
				if (event_data.read_maxsize > 0) chunk = std::min(event_data.read_maxsize, Communication::CHUNK_SIZE);	//	Set the size of the chunk
				if (splice(event_data.fd, NULL, event_data.pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) {				//	Send the first chunk of data from the file to the pipe
					close(event_data.fd); close(event_data.pipe[0]); close(event_data.pipe[1]);							//	Splice failed, close FD's and return
					event->client->remove(); return;
				}
				std::swap(event_data.fd, event_data.pipe[0]);															//	Swap the read end of the pipe with the fd (this is to be consistent with the FD that EPOLL is monitoring)

				Event::events[event_data.fd] = event_data;																//	Add the DATA event to the event's list

				event->write_info = 0;																					//	Set some flags
				event->write_size = 0;																					//	Set some flags
				event->write_maxsize = header.size() + event_data.read_maxsize;											//	Set the total size of the data to be sent
				event->response_size = event_data.read_maxsize;
				event->write_buffer.clear();																			//	Clear write_buffer
				event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());					//	Copy the header to write_buffer

				// if (Epoll::set(event->fd, !(event->header_map["Write-Only"] == "true"), true) == -1)					//	Set EPOLL to monitor write events for the client
				// 	event->client->remove();
				if (Epoll::add(event_data.fd, true, false) == -1) {												//	Set EPOLL to monitor read events for DATA
					event->read_maxsize = 0;																			//	If set EPOLL fails, reset the flag,
					event->write_buffer.clear();																		//	clear writte_buffer
					Event::remove(event_data.fd);																		//	and remove the DATA event
					event->client->remove();
				}

				return;
			}

		#pragma endregion

	#pragma endregion

	#pragma region CGI

		#pragma region Variables

			void Protocol::variables_cgi(EventInfo * event, std::vector<std::string> & cgi_vars) {
				if (!event) return;

				cgi_vars.push_back("CONTENT_LENGTH=" + event->header_map["Content-Length"]);
				cgi_vars.push_back("CONTENT_TYPE=" + event->header_map["Content-Type"]);

				cgi_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");

				cgi_vars.push_back("HTTPS=off");
				cgi_vars.push_back("HTTP_ACCEPT=" + (event->header_map["Accept"].empty() ? "*/*" : event->header_map["Accept"]));
				cgi_vars.push_back("HTTP_HOST=" + event->header_map["$server_name"]);
				cgi_vars.push_back("HTTP_REFERER=" + event->header_map["Referer"]);
				cgi_vars.push_back("HTTP_USER_AGENT=" + event->header_map["User-Agent"]);
				cgi_vars.push_back("HTTP_COOKIE=" + event->header_map["$http_cookie"]);

				cgi_vars.push_back("PATH_INFO=" + event->response_map["Path-Info"]);
				size_t pos = event->response_map["Path"].find_last_of('/');
				if (pos != std::string::npos)	cgi_vars.push_back("PATH_TRANSLATED=/" + event->response_map["Path"].substr(0, pos + 1) + event->response_map["Path-Info"]);
				else							cgi_vars.push_back("PATH_TRANSLATED=/" + event->response_map["Path"]);

				cgi_vars.push_back("QUERY_STRING=" + event->header_map["$query_string"]);

				cgi_vars.push_back("REDIRECT_STATUS=" + Utils::ltos(event->redirect_status));
				cgi_vars.push_back("REMOTE_ADDR=" + event->header_map["$remote_addr"]);
				cgi_vars.push_back("REMOTE_PORT=" + event->header_map["$remote_port"]);

				cgi_vars.push_back("REQUEST_FILENAME=" + event->response_map["Path-Full"]);
				cgi_vars.push_back("REQUEST_METHOD=" + event->header_map["Method"]);
				cgi_vars.push_back("REQUEST_SCHEME=http");
				cgi_vars.push_back("REQUEST_URI=" + event->header_map["$request_uri"]);

				cgi_vars.push_back("SCRIPT_FILENAME=" + event->response_map["Path-Full"]);
				cgi_vars.push_back("SCRIPT_NAME=/" + event->response_map["Path"]);

				cgi_vars.push_back("SERVER_NAME=" + event->header_map["$host"]);
				cgi_vars.push_back("SERVER_PORT=" + event->header_map["$server_port"]);
				cgi_vars.push_back("SERVER_PROTOCOL=" + event->header_map["Protocol"]);
				cgi_vars.push_back("SERVER_SOFTWARE=" + event->response_map["Server"]);
			}
		
			#pragma region Information

				//										EJEMPLO DE UNA SOLICITUD DE UN CLIENTE

				//			POST /cgi-bin/script.cgi/extra/path?item=123&color=red HTTP/1.1
				//			Host: www.example.com
				//			User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.121 Safari/537.36
				//			Referer: https://www.google.com/search?q=webserv
				//			Cookie: sessionid=abcdef1234567890; theme=dark
				//			Accept: text/html,application/xhtml+xml
				//			Content-Type: text/html
				//			Content-Length: 1234

				//	CONTENT_LENGTH		Indica la longitud del cuerpo de la solicitud en bytes. Puede estar vacío									1234
				//	CONTENT_TYPE		Indica el tipo de contenido de los datos. Puede estar vacío													text/html

				//	GATEWAY_INTERFACE	La versión de CGI que está usando el servidor web															CGI/1.1

				//	HTTPS				
				//	HTTP_ACCEPT			Los tipos de contenido que el cliente está dispuesto a aceptar												text/html,application/xhtml+xml
				//	HTTP_HOST			El nombre del servidor virtual que está manejando la solicitud				        						www.example.com
				//	HTTP_REFERER		El valor de referer, que indica la página anterior a la que se hizo la solicitud        					https://www.google.com/search?q=webserv
				//	HTTP_USER_AGENT		El contenido del encabezado user-agent, que identifica el navegador del cliente  							Mozilla/5.0 (Windows NT 10.0; Win64; x64)...

				//	PATH_INFO			Cualquier información adicional después del nombre del script en la URL										/extra/path
				//	PATH_TRANSLATED		La ruta completa en el sistema de archivos del recurso solicitado											/var/www/cgi-bin/extra/path

				//	QUERY_STRING		Es la cadena de consulta (query string), que contiene los parámetros enviados después de ?					item=123&color=red

				//	REDIRECT_STATUS		Código de estado de la anterior redirección. Puede estar vacío												302
				//	REMOTE_ADDR			La dirección IP del cliente que hizo la solicitud					             							203.0.113.45
				//	REMOTE_PORT			El puerto del cliente que hizo la solicitud							        								54321 

				//	REQUEST_FILENAME	Ruta completa del archivo solicitado																		/var/www/cgi-bin/script.cgi
				//	REQUEST_METHOD		El método HTTP utilizado en la solicitud (GET, POST, PUT, DELETE, etc.)		  			            		POST
				//	REQUEST_SCHEME		Esquema de la solicitud (HTTP o HTTPS)																		http
				//	REQUEST_URI			Es la URI completa incluyendo la cadena de consulta (query string)		  									/products/details?item=123&color=red

				//	SCRIPT_NAME			La ruta virtual del script CGI que está siendo ejecutado, tal como se solicitó en la URL					/cgi-bin/script.cgi

				//	SERVER_NAME			El valor del encabezado host, que es el nombre del dominio o la dirección IP solicitada		     	    	www.example.com
				//	SERVER_PORT			El puerto del servidor que está manejando la solicitud					 	               					80
				//	SERVER_PROTOCOL		El protocolo HTTP que está utilizando el cliente															HTTP/1.1
				//	SERVER_SOFTWARE		Nombre y versión del software del servidor																	Webserv/1.0 (Ubuntu 22.04.4 LTS)

				//	El cuerpo de la solicitud se envia al CGI a través del STDIN

				// Ejemplo: execve("/usr/bin/python", ["python", "/path/to/script.cgi"], envp);

			#pragma endregion

		#pragma endregion

		#pragma region Fork

			void Protocol::response_cgi(EventInfo * event) {
				if (!event) return;


				event->cgi_fd = -1;
				int write_fd = -1;
			//	Create the event to write to the CGI
				if (event->header_map["Content-Length"].empty() == false) {
					int write_pipe[2];
					if (pipe(write_pipe) == -1) { event->client->remove(); return; }						//	Create the pipe for CGI (read from it)
					Utils::NonBlocking_FD(write_pipe[0]);													//	Set the read end of the pipe as non-blocking
					Utils::NonBlocking_FD(write_pipe[1]);													//	Set the write end of the pipe as non-blocking

					EventInfo event_write_cgi(write_pipe[1], CGI, NULL, event->client);						//	Create the event for the CGI
					write_fd = write_pipe[0];
					event_write_cgi.pipe[0] = write_pipe[0];
					event_write_cgi.pipe[1] = -1;
					event->cgi_fd = event_write_cgi.fd;
					Event::events[event_write_cgi.fd] = event_write_cgi;									//	Add the CGI event to the event's list
				}

			//	Create the event to read from the CGI
				int read_pipe[2];
				if (pipe(read_pipe) == -1) { event->client->remove(); return; }								//	Create the pipe for CGI (read from it)
				Utils::NonBlocking_FD(read_pipe[0]);														//	Set the read end of the pipe as non-blocking
				Utils::NonBlocking_FD(read_pipe[1]);														//	Set the write end of the pipe as non-blocking

				EventInfo event_read_cgi(read_pipe[0], CGI, NULL, event->client);							//	Create the event for the CGI
				event_read_cgi.pipe[0] = -1;
				event_read_cgi.pipe[1] = read_pipe[1];

				Event::events[event_read_cgi.fd] = event_read_cgi;											//	Add the CGI event to the event's list

			//	Set EPOLL
				event->write_info = 0;
				event->write_size = 0;
				event->write_maxsize = 0;
				event->write_buffer.clear();																//	Clear write_buffer

				if (event->cgi_fd != -1 && Epoll::add(event->cgi_fd, false, true) == -1) {					//	Set EPOLL to monitor write events for CGI
					event->client->remove(); return;
				}

				if (Epoll::add(event_read_cgi.fd, true, false) == -1) {										//	Set EPOLL to monitor read events for CGI
					event->client->remove(); return;
				}

				//Fork the process
				event->pid = fork();
				if (event->pid == -1) {
					event->client->remove(); return;
				} else if (event->pid == 0) {
					{
						Epoll::close();

						std::vector<char *> env_array;
						std::vector<std::string> cgi_vars;
						variables_cgi(event, cgi_vars);

						char * args[3];
						if (event->response_map["CGI-Path"] == "Self-CGI") {
							args[0] = const_cast<char *>(event->response_map["Path-Full"].c_str());
							args[1] = NULL;
							args[2] = NULL;
						} else {
							args[0] = const_cast<char *>(event->response_map["CGI-Path"].c_str());
							args[1] = const_cast<char *>(event->response_map["Path-Full"].c_str());
							args[2] = NULL;
						}

						for (size_t i = 0; i < cgi_vars.size(); ++i)
							env_array.push_back(const_cast<char*>(cgi_vars[i].c_str()));
						env_array.push_back(NULL);

						if (write_fd != -1) dup2(write_fd, STDIN_FILENO); else close(STDIN_FILENO);
						dup2(event_read_cgi.pipe[1], STDOUT_FILENO);

						int dev_null = open("/dev/null", O_WRONLY);
						if (dev_null != -1) { dup2(dev_null, STDERR_FILENO); close(dev_null); }

						for (std::map <int, EventInfo>::iterator it = Event::events.begin(); it != Event::events.end(); ++it) {
							if (it->second.fd) close(it->second.fd);
							if (it->second.pipe[0]) close(it->second.pipe[0]);
							if (it->second.pipe[1]) close(it->second.pipe[1]);
						}

						execve(args[0], args, &env_array[0]);
					}
					exit(1);
				}
				event->redirect_status = 0;
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Process

	void Protocol::process_response(EventInfo * event) {
		if (!event) return;

		if 		(event->response_method == "Error")		response_error(event);
		else if (event->response_method == "Redirect")	response_redirect(event);
		else if (event->response_method == "Directory")	response_directory(event);
		else if (event->response_method == "File")		response_file(event);
		else if (event->response_method == "CGI")		response_cgi(event);
	}

#pragma endregion
