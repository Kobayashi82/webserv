/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 11:52:00 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/29 12:56:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Socket.hpp"
#include "Event.hpp"
#include "Epoll.hpp"
#include "Protocol.hpp"
#include "Communication.hpp"

	//	TODO	Index usa index.html por defecto
	//	TODO	Update resource path with alias or any modified path before cgi

	//	*	127.0.0.1 not receiving logs

	//	*	Error tiene que comprobar locations
	//	*	Return, error y try_files manejan variables
	//	*	Location y try_files pueden llevar = (try_files para códigos =404)

#pragma region Variables

	bool		Protocol::internal = false;																//	
	std::string	Protocol::internal_path;																//	

#pragma endregion

#pragma region Parsers

	#pragma region Directives

		#pragma region Set File

			void set_file(EventInfo * event, std::string & path) {
				event->response_map["Method"] = "File";
				event->response_map["Code"] = "200";
				event->response_map["Path-Full"] = path;

				size_t pos = event->response_map["Path-Full"].find_last_of('.');
				if (pos != std::string::npos) event->response_map["Content-Type"] = Settings::mime_types[event->response_map["Path-Full"].substr(pos + 1)];
				if (event->response_map["Content-Type"].empty()) event->response_map["Content-Type"] = "application/octet-stream";
			}

		#pragma endregion

		#pragma region File Stat

			bool file_stat(EventInfo * event, const std::string & path) {
				struct stat path_stat;

				event->filesize = 0;																		//	Reset 'filesize'
				event->mod_time = time(NULL);																//	Reset 'mod_time'

				if (stat(path.c_str(), &path_stat) != 0) return (false);									//	Check if the file exists
				if (!(path_stat.st_mode & S_IRUSR)) return (false);											//	Check if the file has read permissions

				event->mod_time = path_stat.st_mtime;														//	Set the file's last modification time
				event->filesize = path_stat.st_size;														//	Set the file size

				char date[30];
				struct tm* timeInfo = gmtime(&event->mod_time);
				strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);	
				event->response_map["Last-Modified"] = date;

				return (true);
			}

		#pragma endregion

		#pragma region Error Page

			int error_page(EventInfo * event, std::string code, VServer * VServ, Location * Loc = NULL) {
				std::string temp, root;

				if (Loc) root = Loc->get("root");
				if (root.empty() && VServ) root = VServ->get("root");
				if (root.empty()) root = Settings::global.get("root");

				if (Loc) temp = Loc->get("error_page " + code);
				if (temp.empty() && VServ) temp = VServ->get("error_page " + code);
				if (temp.empty()) temp = Settings::global.get("error_page " + code);

				if (!VServ) VServ = &Settings::global;

				std::string path = Utils::fullpath(root + "/" + temp);
				if (temp.empty() || (!Communication::cache.exists(path) && !file_stat(event, path))) {
					event->response_map["Method"] = "Error";
					event->response_map["Code"] = code;
				} else {
					event->response_map["Method"] = "File";
					event->response_map["Code"] = code;
					event->response_map["Path-Full"] = path;
				}

				event->VServ = VServ; event->Loc = Loc;
				if (Loc)		event->vserver_data = &Loc->data;
				else if (VServ)	event->vserver_data = &VServ->data;

				event->response_map["Content-Type"] = "text/html";

				return (1);
			}

		#pragma endregion

		#pragma region Redirect

			static int redirect(EventInfo * event, std::string code_path, VServer * VServ, Location * Loc = NULL) {
				std::string code, path;

				size_t pos = code_path.find_first_of(' ');
				if (pos == std::string::npos) return (0);

				code = code_path.substr(0, pos);
				path = code_path.substr(pos + 1);

				event->response_map["Method"] = "Redirect";
				event->response_map["Code"] = code; // 301, 302, 303, 307, 308
				event->response_map["Path"] = path;

				if (!VServ) VServ = &Settings::global;

				event->VServ = VServ; event->Loc = Loc;
				if (Loc)		event->vserver_data = &Loc->data;
				else if (VServ)	event->vserver_data = &VServ->data;

				return (1);
			}

		#pragma endregion

		#pragma region CGI

			#pragma region Method

				int cgi_method(EventInfo * event, std::string cgi, std::string cgi_path, VServer * VServ, Location * Loc = NULL) {
					std::string root;

					if (Loc)						root = Loc->get("root");
					if (root.empty() && VServ)		root = VServ->get("root");
					if (root.empty())				root = Settings::global.get("root");
					
					size_t pos = cgi.find_first_of(' ');
					if (pos == std::string::npos) return (0);
					cgi = Utils::strToLower(cgi.substr(pos + 1));

					if (cgi_path.empty()) return (0);
					if (cgi_path[0] != '/' && cgi_path[0] != '~')	cgi_path = Utils::fullpath(Settings::program_path + cgi_path);
					else											cgi_path = Utils::fullpath(cgi_path);

					if (Utils::file_exists(cgi_path, true) == 1) return (error_page(event, "404", VServ, Loc));
					if (Utils::file_exists(cgi_path, true) == 2) return (error_page(event, "403", VServ, Loc));

					if (cgi == "head" || cgi == "get" || cgi == "post" || cgi == "put" || cgi == "patch" || cgi == "delete") {
						if (cgi == Utils::strToLower(event->header_map["Method"])) {
							event->response_map["Method"] = "CGI";
							event->response_map["Path-Full"] = cgi_path;
							event->response_map["CGI-Path"] = "Self-CGI";
							return (1);
						}
						return (0);
					}

					return (0);
				}

			#pragma endregion

			#pragma region Directory

				int cgi_dir(EventInfo * event, VServer * VServ, Location * Loc = NULL) {
					std::string root, cgi_path;

					if (Loc)							cgi_path = Loc->get("cgi dir");
					if (cgi_path.empty() && VServ)		cgi_path = VServ->get("cgi dir");
					if (cgi_path.empty())				cgi_path = Settings::global.get("cgi dir");
					if (cgi_path.empty()) return (0);

					if (Loc)						root = Loc->get("root");
					if (root.empty() && VServ)		root = VServ->get("root");
					if (root.empty())				root = Settings::global.get("root");
					
					if (cgi_path[0] != '/' && cgi_path[0] != '~')	cgi_path = Utils::fullpath(Settings::program_path + cgi_path);
					else											cgi_path = Utils::fullpath(cgi_path);

					if (Utils::file_exists(cgi_path, true) == 1) return (error_page(event, "404", VServ, Loc));
					if (Utils::file_exists(cgi_path, true) == 2) return (error_page(event, "403", VServ, Loc));

					event->response_map["Method"] = "CGI";
					event->response_map["Path-Full"] = cgi_path;
					event->response_map["CGI-Path"] = "Self-CGI";

					return (1);
				}

			#pragma endregion

			#pragma region Extension

				int cgi_ext(EventInfo * event, VServer * VServ, Location * Loc = NULL) {
					std::string root, cgi_path;

					std::string ext = Utils::strToLower(event->response_map["Path"]);
					size_t pos = ext.find_last_of('.');
					if (pos == std::string::npos) return (0);
					ext = ext.substr(pos); if (ext.empty()) return (0);

					if (Loc)							cgi_path = Loc->get("cgi " + ext);
					if (cgi_path.empty() && VServ)		cgi_path = VServ->get("cgi " + ext);
					if (cgi_path.empty())				cgi_path = Settings::global.get("cgi " + ext);
					if (cgi_path.empty()) return (0);

					if (Loc)							root = Loc->get("root");
					if (root.empty() && VServ)			root = VServ->get("root");
					if (root.empty())					root = Settings::global.get("root");

					if (Utils::strToLower(cgi_path) != "self-cgi") {
						if (cgi_path[0] != '/' && cgi_path[0] != '~')	cgi_path = Utils::fullpath(Settings::program_path + cgi_path);
						else											cgi_path = Utils::fullpath(cgi_path);

						if (Utils::file_exists(cgi_path, true) == 1) return (error_page(event, "404", VServ, Loc));
						if (Utils::file_exists(cgi_path, true) == 2) return (error_page(event, "403", VServ, Loc));
					}

					event->response_map["Method"] = "CGI";
					event->response_map["Path-Full"] = Utils::fullpath(root + "/" + event->response_map["Path"]);
					if (Utils::file_exists(event->response_map["Path-Full"]) == 1) return (error_page(event, "404", VServ, Loc));
					if (Utils::file_exists(event->response_map["Path-Full"]) == 2) return (error_page(event, "403", VServ, Loc));
					event->response_map["CGI-Path"] = cgi_path;

					return (1);
				}

			#pragma endregion

		#pragma endregion

		#pragma region Directory

			int check_dir(EventInfo * event, VServer * VServ, Location * Loc = NULL) {
				std::string root;

				if (root.empty() && Loc)	root = Loc->get("root");
				if (root.empty() && VServ)	root = VServ->get("root");
				if (root.empty())			root = Settings::global.get("root");

				std::string path = event->response_map["Path"];

				if (chdir((root + "/" + path).c_str()) != 0) return (0);
				
				if (path.empty()) path = "/";

				//if (Utils::directory_exists(path)) return (0);

				std::string index;
				if (index.empty() && Loc)	index = Loc->get("index");
				if (index.empty() && VServ)	index = VServ->get("index");
				if (index.empty())			index = Settings::global.get("index");
				if (index.empty())			index = "index.html";

				std::istringstream iss(index);

				while (iss >> index) {
					path = event->response_map["Path"] + "/" + index;
					if (!Utils::file_exists(path)) {
						if (!cgi_ext(event, VServ, Loc)) set_file(event, path);
						return (1);
					}
				}

				std::string autoindex;
				if (Loc)							autoindex = Loc->get("autoindex");
				if (autoindex.empty() && VServ)		autoindex = VServ->get("autoindex");
				if (autoindex.empty())				autoindex = Settings::global.get("autoindex");

				if (autoindex != "on") return (0);

				path = event->response_map["Path"];
				if (path.empty()) path = "/";

				if (!cgi_dir(event, VServ, Loc)) {
					event->response_map["Method"] = "Directory";
					event->response_map["Code"] = "200";
					event->response_map["Path-Full"] = path;
				}

				return (1);
			}

		#pragma endregion

		#pragma region File

			int check_file(EventInfo * event, VServer * VServ, Location * Loc = NULL) {
				std::string root;

				if (root.empty() && Loc)	root = Loc->get("root");
				if (root.empty() && VServ)	root = VServ->get("root");
				if (root.empty())			root = Settings::global.get("root");

				if (chdir(root.c_str()) != 0) return (0);

				std::string path = event->response_map["Path"];
				if (!Utils::file_exists(path)) {
					if (!cgi_ext(event, VServ, Loc)) set_file(event, path);
					return (1);
				}

				return (0);
			}

		#pragma endregion

		#pragma region Method

			int method(EventInfo * event, int index, VServer * VServ, Location * Loc = NULL) {
				Method * Met = NULL;

				if (Loc) Met = &Loc->method[index];
				if (!Met && VServ) Met = &VServ->method[index];
				if (!Met) Met = &Settings::global.method[index];
				if (!Met || Met->get("method").find(event->header_map["Method"]) == std::string::npos) return (0);

				bool allowed = false;

				std::vector<std::pair<std::string, std::string> >::const_iterator it;
				for (it = Met->data.begin(); it != Met->data.end(); ++it) {
					if (it->first == "allow" && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) allowed = true;
					if (it->first == "deny" && allowed == false && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) return (error_page(event, "403", VServ));
					if (it->first == "return") return (redirect(event, it->second, VServ, Loc));
				}

				return (0);
			}

		#pragma endregion

		#pragma region Location

			int location(EventInfo * event, int index, VServer * VServ) {
				Location * Loc = NULL;

				if (VServ) Loc = &VServ->location[index];
				if (!Loc) return (0);

				event->VServ = VServ; event->Loc = Loc; event->vserver_data = &Loc->data;

				bool allowed = false;

				std::vector<std::pair<std::string, std::string> >::const_iterator it;
				for (it = Loc->data.begin(); it != Loc->data.end(); ++it) {
					if (it->first == "allow" && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) allowed = true;
					if (it->first == "deny" && allowed == false && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) return (error_page(event, "403", NULL));
					if (it->first == "return") return (redirect(event, it->second, NULL));
					//if (it->first == "try_files") return (redirect(event, it->second, NULL));
					if (it->first == "method" && method(event, Utils::sstol(it->second), NULL)) return (1);
				}

				event->VServ = VServ; event->Loc = NULL; event->vserver_data = &VServ->data;

				return (0);
			}

		#pragma endregion

		#pragma region VServers

			bool isAddress(VServer * VServ, const std::string & ip, int port) {
				std::vector<std::pair<std::string, int> >::const_iterator it;
				for (it = VServ->addresses.begin(); it != VServ->addresses.end(); ++it) {
					if (it->first == "0.0.0.0" && it->second == port) return (true);
					if (it->first == ip && it->second == port) return (true);
				}

				return (false);
			}

			int server(EventInfo * event, VServer * VServ) {
				Location * Loc = NULL;	int Loc_index = -1;
										int counter = 0;

				std::deque<Location>::iterator l_it;
				for (l_it = VServ->location.begin(); l_it != VServ->location.end(); ++l_it) {
					Location * tmp_loc = &(*l_it);

					(void) Loc;
					(void) Loc_index;
					(void) tmp_loc;
					// bool valid_name = false;
					// std::string server_name; std::istringstream iss(Utils::strToLower(tmp_loc->get("server_name")));
					// while (iss >> server_name)
					// 	if (server_name == Utils::strToLower(event->header_map["Host"])) { Loc = tmp_loc; Loc_index = counter; valid_name = true; break; }
					// if (valid_name) break;

					counter++;
				}

				event->VServ = VServ; event->Loc = NULL; event->vserver_data = &VServ->data;

				bool allowed = false;

				std::vector<std::pair<std::string, std::string> >::const_iterator it;
				for (it = VServ->data.begin(); it != VServ->data.end(); ++it) {
					if (it->first == "allow" && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) allowed = true;
					if (it->first == "deny" && allowed == false && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) return (error_page(event, "403", VServ));
					if (it->first == "return") return (redirect(event, it->second, VServ));
					if (it->first == "method" && method(event, Utils::sstol(it->second), VServ)) return (1);
				}

				event->VServ = &Settings::global; event->Loc = NULL; event->vserver_data = &Settings::global.data;

				return (0);
			}

		#pragma endregion

		#pragma region Global

			int global(EventInfo * event) {
				VServer * default_VServ = NULL;	int default_index = -1;
				VServer * VServ = NULL;			int VServ_index = -1;
												int counter = 0;

				std::deque<VServer>::iterator v_it;
				for (v_it = Settings::vserver.begin(); v_it != Settings::vserver.end(); ++v_it) {
					VServer * tmp_serv = &(*v_it);

					if (v_it->bad_config || v_it->force_off || isAddress(tmp_serv, event->socket->ip, event->socket->port) == false) continue;
					if (!default_VServ) { default_VServ = tmp_serv; default_index = counter; }

					bool valid_name = false;
					std::string server_name; std::istringstream iss(Utils::strToLower(tmp_serv->get("server_name")));
					while (iss >> server_name)
						if (server_name == Utils::strToLower(event->header_map["Host"])) { VServ = tmp_serv; VServ_index = counter; valid_name = true; break; }
					if (valid_name) break;
					counter++;
				}

				if (!VServ) { VServ = default_VServ; VServ_index = default_index; }

				event->VServ = &Settings::global; event->Loc = NULL; event->vserver_data = &Settings::global.data;
				
				bool allowed = false;

				std::vector<std::pair<std::string, std::string> >::const_iterator it;
				for (it = Settings::global.data.begin(); it != Settings::global.data.end(); ++it) {
					if (it->first == "allow" && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) allowed = true;
					if (it->first == "deny" && allowed == false && (it->second == "all" || Utils::isIPInRange(event->client->ip, it->second))) return (error_page(event, "403", NULL));
					if (it->first == "return") return (redirect(event, it->second, NULL));
					if (it->first.substr(0, 3) == "cgi" && cgi_method(event, it->first, it->second, NULL)) return (1);
					if (it->first == "method" && method(event, Utils::sstol(it->second), NULL)) return (1);
					if (it->first == "server" && Utils::sstol(it->second) == VServ_index && server(event, VServ)) return (1);
				}

				if (check_file(event, NULL)) return (1);
				if (check_dir(event, NULL)) return (1);

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region Request

		void Protocol::parse_request(EventInfo * event) {
			if (!event) return;

		//	Retrieve the request path without the query string
			std::string path = event->header_map["$uri"];																					//	Request path without the query string
			size_t dot_pos = path.find('.');
			size_t slash_pos = path.find('/', dot_pos);

			event->response_map["Path-Full"] = "";
			if (dot_pos != std::string::npos && slash_pos != std::string::npos) {															//	THIS MUST BE CHECKED
				event->response_map["Path"] = path.substr(1, slash_pos);
				event->response_map["Path-Info"] = path.substr(slash_pos);
			} else {
				event->response_map["Path"] = path.substr(1);
				event->response_map["Path-Info"] = "";
			}

		//	General header information
			event->response_map["Protocol"] = "HTTP/1.1";																					//	Protocol of the connection (by default HTTP/1.1)
			event->response_map["Connection"] = event->header_map["Connection"];
			if (event->response_map["Connection"].empty()) event->response_map["Connection"] = "keep-alive";								//	Respect the client's request if indicated
			
			event->response_map["Server"] = Settings::server_name + "/" + Settings::server_version + Settings::os_name;						//	Name and version of the server
			event->response_map["Date"] = Settings::timer.current_time_header();															//	Current date in GMT format


		//	Unknown request methods return an error
			if (std::string("HEAD|GET|POST|PUT|PATCH|DELETE").find(event->header_map["Method"]) == std::string::npos)
				error_page(event, "405", event->VServ, event->Loc);																			//	Return "405 (Method Not Allowed)"
			else if (!global(event) || event->response_map["Code"].empty())																	//	If it is not a valid response...
				error_page(event, "500", event->VServ, event->Loc);																			//	Return "500 (Internal Server Error)"

		//	Only HEAD and GET are valid without a CGI
			if (event->response_map["Method"] != "CGI" && event->header_map["Method"] != "HEAD" && event->header_map["Method"] != "GET")
				error_page(event, "403", event->VServ, event->Loc);																			//	Return "403 (Forbidden)"

		//	Check if response method is unknown (not really a case, but whatever...)
			event->response_method = event->response_map["Method"];
			if (event->response_method != "File" && event->response_method != "CGI" && event->response_method != "Directory"
				&& event->response_method != "Redirect" && event->response_method != "Error") {
					error_page(event, "403", event->VServ, event->Loc);																		//	Return "403 (Forbidden)"
			}

		//	Check the maximum body size against the request's 'Content-Length'
			std::string									body_maxsize;
			if (event->Loc)								body_maxsize = event->Loc->get("body_maxsize");
			if (body_maxsize.empty() && event->VServ)	body_maxsize = event->VServ->get("body_maxsize");									//	Get 'body_maxsize'
			if (!body_maxsize.empty()) {
				event->body_maxsize   =	Utils::sstol(body_maxsize);																			//	Get 'body_maxsize' in numeric format
				size_t content_length =	Utils::sstol(event->header_map["Content-Length"]);													//	Get 'Content-Length' in numeric format
				if (event->body_maxsize > 0 && content_length > event->body_maxsize)														//	If 'Content-Length' is greater than 'body_maxsize'...
					error_page(event, "403", event->VServ, event->Loc);																		//	Return "403 (Forbidden)"
			}

		//	Response method is a file, check if must be cached
			if (event->response_method == "File") {
				if (!Communication::cache.exists(event->response_map["Path-Full"]) && !file_stat(event, event->response_map["Path-Full"]))	//	If not in cache and the file doesn't exist or can't be read...
					error_page(event, "404", event->VServ, event->Loc);																		//	Return "404 (Not Found)"
				else {
					size_t pos1 = event->header_map["Cache-Control"].find("no-cache");
					size_t pos2 = event->header_map["Cache-Control"].find("no-store");
					event->no_cache = (pos1 != std::string::npos || pos2 != std::string::npos);												//	Determine whether the file should be added to the cache
				}
			}

		//	Add the 'Code-Description' to the response
			event->response_map["Code-Description"] = Settings::error_codes[Utils::sstol(event->response_map["Code"])];
		}

		#pragma region Information

			//	Directivas tienen preferencias de orden

			//	List of VServer that can manage the ip:port of the conection, aka event->socket->IP & port. Must be enabled
			//	Get the VServer that manage the request
			//	Get the location that manage the request
			//	Get the limit_except that manage the request

			//	If IP is deny.		code 403
			//	If method deny.		code 405
			//	If return.			return url			(code and url is in the configuration and the response is the same for local path or url. encode url.)
			//	If error.			return error		(check inversed if error is manage in VServer or global and if it is a file, check if exists and permissions)
			//	If directory.		return directory	(code 200. check if exists and permissions)
			//	If file.			return file			(code 200. check if exists and permissions)

			//	Also return the path for access_log, error_log and theri maxsizes (if aplicable)



			//	------------------------------
			//	INFORMACION DE LOS ENCABEZADOS
			//	------------------------------

			//	Protocol 				HTTP/1.1
			//	Type					error, redirection, file, directory, cgi
			//	Path					ruta de la redirección, archivo, carpeta o cgi	(incluye el "Location" en caso de redireccion)	(incluye el "Allow" en caso de error 405 Method Not Allowd)
			//	Code					código de la respuesta (200, 404, etc...)
			//	CodeInfo				descripción del código
			//	Date					fecha actual (Tue, 12 Sep 2024 15:44:18 GMT)
			//	Server					Webserv/1.0
			//	X-Content-Type-Options	nosniff
			//	Connection:				keep-alive (close solamente si el cliente lo solicita)
			//	Content-Type			tipo de contendio (text/html; charset=UTF-8)  charset=UTF-8 solo si el tipo es "text/"
			//	Content-Length:			tamaño del cuerpo del mensaje en bytes



			//	Protocol:				HTTP/1.1
			//	Code:					200
			//	Code_Info:				OK

			//	Date:					Tue, 12 Sep 2024 15:44:18 GMT
			//	Server:					Webserv/1.0
			//	X-Content-Type-Options:	nosniff
			//	Connection:				close or keep-alive
			//	Content-Type:			text/html; charset=UTF-8
			//	Content-Length:			1234

			//	Location:				http://www.new-url.com/		(para redirecciones)
			//	Allow:					GET, POST					(para metodos no allowed)
			//	Cache-Control
			//	Last-Modified

			//?	ARCHIVO ESTATICO
			//?	----------------
			//	HTTP/1.1 200 OK
			//	Date: Tue, 12 Sep 2024 15:44:18 GMT
			//	Server: Webserv/1.0
			//	Content-Type: text/html; charset=UTF-8
			//	Content-Length: 1234
			//	Connection: close

			//	<HTML content goes here...>

			//?	ERROR
			//?	-----
			//	HTTP/1.1 404 Not Found
			//	Date: Tue, 12 Sep 2024 15:44:18 GMT
			//	Server: Webserv/1.0
			//	Content-Type: text/html; charset=UTF-8
			//	Content-Length: 88
			//	Connection: close

			//	<html><body><h1>404 Not Found</h1><p>The requested resource could not be found.</p></body></html>

			//?	REDIRECCION
			//?	-----------
			//	HTTP/1.1 301 Moved Permanently
			//	Date: Tue, 12 Sep 2024 15:44:18 GMT
			//	Server: MyCustomServer/1.0
			//	Location: http://www.new-url.com/
			//	Content-Length: 0
			//	Connection: close

			//?	DIRECTORIO
			//?	----------
			//	HTTP/1.1 200 OK
			//	Date: Tue, 12 Sep 2024 15:44:18 GMT
			//	Server: MyCustomServer/1.0
			//	Content-Type: text/html; charset=UTF-8
			//	Content-Length: 456
			//	Connection: close

			//	<html><body><h1>Index of /dir</h1><ul><li><a href="file1.txt">file1.txt</a></li><li><a href="file2.jpg">file2.jpg</a></li></ul></body></html>

			//?	PROTOCOLO NO SOPORTADO (HTTP/2.0)
			//?	---------------------------------
			//	HTTP/1.1 505 HTTP Version Not Supported
			//	Content-Type: text/plain
			//	Content-Length: 30

			//	HTTP/2.0 Protocol Not Supported

			//?	IP DENEGADA
			//?	-----------
			//	HTTP/1.1 403 Forbidden
			//	Date: Fri, 13 Sep 2024 15:44:18 GMT
			//	Server: Webserv/1.0
			//	Content-Type: text/html; charset=UTF-8
			//	Content-Length: 1234

			//	<html>
			//	<head><title>403 Forbidden</title></head>
			//	<body>
			//	<h1>403 Forbidden</h1>
			//	<p>You don't have permission to access / on this server.</p>
			//	</body>
			//	</html>

		#pragma endregion

	#pragma endregion

	#pragma region Code

		int Protocol::parse_code(EventInfo * event, int code) {
			(void) event;
			(void) code;

			//	Check if there are operations with the code and send the response to the cliente
			//	Use for the return of a CGI
			return (0);
		}

	#pragma endregion

	#pragma region Header

		#pragma region Variables

			void Protocol::parse_variables(EventInfo * event) {
				if (!event) return;

				std::string path = event->header_map["Path"];
				size_t pos = std::min(path.find_first_of('?'), path.size());

				event->header_map["$request_uri"] = event->header_map["Method"] + " " + path + " " + event->header_map["Protocol"];

				event->header_map["$uri"] = path.substr(0, pos);
				event->header_map["$document_uri"] = event->header_map["$uri"];

				if (pos != path.size()) event->header_map["$args"] = path.substr(pos + 1);
				event->header_map["$query_string"] = event->header_map["$args"];

				event->header_map["$remote_addr"] = event->client->ip;
				event->header_map["$remote_port"] = Utils::ltos(event->client->port);
				if (event->type == CLIENT) {
					event->header_map["$server_addr"] = event->socket->ip;
					event->header_map["$server_port"] = Utils::ltos(event->socket->port);
				}

				event->header_map["$server_name"] = event->header_map["Host"];									//	El nombre del dominio que lo maneja o el nombre del primer dominio por defecto
				if (event->header_map["$server_name"].empty()) event->header_map["$server_name"] = event->header_map["$server_addr"];

				event->header_map["$http_referer"] = event->header_map["Referer"];
				event->header_map["$http_cookie"] = event->header_map["Cookie"];
				event->header_map["$http_user_agent"] = event->header_map["User-Agent"];
				event->header_map["$http_host"] = event->header_map["Host"];

				event->header_map["$host"] = event->header_map["Host"];
				if (event->header_map["$host"].empty()) event->header_map["$host"] = event->header_map["$server_name"];
				if (event->header_map["$host"].empty()) event->header_map["$host"] = event->header_map["$server_addr"];
			}


			//										EJEMPLO DE UNA SOLICITUD DE UN CLIENTE

			//			GET /products/details?item=123&color=red HTTP/1.1
			//			Host: www.example.com
			//			User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.121 Safari/537.36
			//			Referer: https://www.google.com/search?q=webserv
			//			Cookie: sessionid=abcdef1234567890; theme=dark

			//	$request				La solicitud completa.				               															GET/products/details?item=123&color=red HTTP/1.1
			//	$request_method			El método HTTP utilizado en la solicitud (GET, POST, PUT, DELETE, etc.)	  			        	    		GET
			//	$request_uri			Es la URI completa incluyendo la cadena de consulta (query string)		  									/products/details?item=123&color=red
			//	$uri, $document_uri		Es la URI sin incluir la cadena de consulta (query string)				 	  								/products/details
			//	$args, $query_string	Es la cadena de consulta (query string), que contiene los parámetros enviados después de ?					item=123&color=red
			//	$host					El nombre del host solicitado. Si no se especifica, se usa server_name o la dirección IP del servidor		www.example.com
			//	$remote_addr			La dirección IP del cliente que hizo la solicitud					             							203.0.113.45
			//	$remote_port			El puerto del cliente que hizo la solicitud							        								54321 
			//	$server_addr			La dirección IP del servidor que está manejando la solicitud				             					192.168.1.10
			//	$server_port			El puerto del servidor que está manejando la solicitud					 	               					80
			//	$server_name			El nombre del servidor virtual que está manejando la solicitud				        						www.example.com
			//	$http_referer			El valor de referer, que indica la página anterior a la que se hizo la solicitud        					https://www.google.com/search?q=webserv
			//	$http_cookie			El valor de la cookie enviada en la solicitud HTTP															sessionid=abcdef1234567890; theme=dark
			//	$http_host				El valor del encabezado host, que es el nombre del dominio o la dirección IP solicitada		       		  	www.example.com
			//	$http_user_agent		El contenido del encabezado user-agent, que identifica el navegador del cliente  							Mozilla/5.0 (Windows NT 10.0; Win64; x64)...

		#pragma endregion

		#pragma region Header

			int Protocol::parse_header(EventInfo * event) {
				if (!event) return (2);

				if (event->read_buffer.empty()) return (1);

				std::string header = std::string(event->read_buffer.begin(), event->read_buffer.end());				//	Create a string with the data read
				size_t pos = header.find("\r\n\r\n");																//	Find the end of the header

				if (pos == std::string::npos)	return (1);															//	Incomplete header
				else							event->header = header.substr(0, pos);								//	Get only the header content

				std::istringstream stream(header); std::string line;

				if (std::getline(stream, line)) {																	//	Read the first line
					std::istringstream first_line(line);
					std::string value1, value2, value3;


					if (event->type == CLIENT) {
						if (first_line >> value1 >> value2 >> value3) {												//	Get the data from the first line (Method, Path and Protocol)
							event->header_map["Method"] = value1;
							event->header_map["Path"] = Security::decode_url(value2);
							event->header_map["Protocol"] = value3;
						} else return (2);																			//	There are errors in the first line
					} if (event->type == CGI) {
						if (first_line >> value1 >> value2 >> value3) {												//	Get the data from the first line (Protocol, Code and Code description)
							event->header_map["Protocol"] = value1;
							event->header_map["Code"] = value2;
							event->header_map["Code-Description"] = value3;

							if (parse_code(event, Utils::sstol(value2))) return (3);								//	The server will manage the response
						} else return (2);																			//	There are errors in the first line
					}
				}

				while (std::getline(stream, line) && line != "\r") {												//	Read the lines (ignoring '\r')
					line.erase(line.size() - 1, 1);
					pos = line.find(':');																			//	Find ':' to split Key - Value
					if (pos != std::string::npos) event->header_map[line.substr(0, pos)] = line.substr(pos + 1);	//	Add the Key - Value to 'header_map'

					if (event->type == CLIENT) parse_variables(event);												//	Create header variables
				}
				pos = event->header_map["Host"].find_first_of(':');
				if (pos != std::string::npos) event->header_map["Host"] = Utils::strim(event->header_map["Host"].substr(0, pos));

			return (0);
		}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region Process

	void Protocol::process_request(EventInfo * event) {
		if (!event) return;

		gettimeofday(&event->response_time, NULL);														//	Reset response time

		parse_request(event);
		process_response(event);
	}

#pragma endregion
