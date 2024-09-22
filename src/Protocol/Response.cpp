/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/21 11:59:50 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/22 19:28:15 by vzurera-         ###   ########.fr       */
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

		void Protocol::response_error(EventInfo * event) {
			if (!event) return;

			std::string code = event->response_map["code"];
			std::string description = event->response_map["code_description"];

			std::string body =
				"<html>\n"
				"<head>\n"
				"    <title>" + code + " (" + description + ")</title>\n"
				"    <style>\n"
				"        body { text-align: center; font-family: Arial, sans-serif; }\n"
				"        h1 { font-size: 100px; margin: 20px; }\n"
				"        h2 { font-size: 50px; margin: 10px; }\n"
				"    </style>\n"
				"</head>\n"
				"<body>\n"
				"    <h1>" + code + "</h1>\n"
				"    <h2>" + description + "</h2>\n"
				"</body>\n"
				"</html>";

			std::string header =
				event->response_map["Protocol"] + " " + code + " " + description + "\r\n"
				"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
				"X-Content-Type-Options: nosniff\r\n"
				"Content-Length: " + Utils::ltos(body.size()) + "\r\n"
				"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

			event->file_info = 0;																	//	Set some flags
			event->file_read = 0;																	//	Set some flags
			event->file_size = 0;																	//	Set some flags
			event->response_map["header"] = header;
			event->write_buffer.clear();															//	Clear write_buffer
			event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());	//	Copy the header to write_buffer

			if (event->response_map["method"] != "HEAD") {
				event->response_map["body"] = body;
				event->response_size = body.size();
				event->file_size = header.size() + body.size();											//	Set the total size of the data to be sent
				event->write_buffer.insert(event->write_buffer.end(), body.begin(), body.end());		//	Copy the body to write_buffer
			}

			if (Epoll::set(event->fd, true, true) == -1) event->client->remove();					//	Set EPOLL to monitor write events
		}

	#pragma endregion

	#pragma region Redirect

		void Protocol::response_redirect(EventInfo * event) {
			if (!event) return;

			std::string header =
				event->response_map["Protocol"] + " " + event->response_map["code"] + " " + event->response_map["code_description"] + "\r\n"
				"Location: " + event->response_map["path"] + "\r\n"
				"X-Content-Type-Options: nosniff\r\n"
				"Content-Length: 0\r\n"
				"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

			event->response_map["header"] = header;
			event->file_info = 0;																	//	Set some flags
			event->file_read = 0;																	//	Set some flags
			event->response_size = 0;
			event->file_size = header.size();														//	Set the total size of the data to be sent
			event->write_buffer.clear();															//	Clear write_buffer
			event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());	//	Copy the header to write_buffer
			if (Epoll::set(event->fd, true, true) == -1) event->client->remove();					//	Set EPOLL to monitor write events
		}

	#pragma endregion

	#pragma region Directory

		bool is_directory(const std::string & path) {
			struct stat statbuf;

			if (stat(path.c_str(), &statbuf) != 0) return (false);
			return (S_ISDIR(statbuf.st_mode));
		}

		void Protocol::response_directory(EventInfo * event) {
			std::vector<std::string> files;
			std::vector<std::string> directories;
			
			std::string dir_path = event->response_map["path"];

			DIR *dir = opendir(dir_path.c_str());											// Abrir el directorio
			if (!dir) return; // return error?
			
			struct dirent *entry;
			while ((entry = readdir(dir)) != NULL) {										// Leer los contenidos del directorio
				std::string name = entry->d_name;

				if (name == "." || name == "..") continue;									// Ignorar . y ..

				std::string full_path = dir_path + "/" + name;
				if (is_directory(full_path))	directories.push_back(name + "/");
				else							files.push_back(name);
			}

			closedir(dir);

			std::string body =
				"<!DOCTYPE html>\n"
				"<html lang=\"en\">\n<head>\n"
				"<meta charset=\"UTF-8\">\n"
				"<title>Index of " + dir_path + "</title>\n"
				"</head>\n<body>\n"
				"<h1>Index of " + dir_path + "</h1>\n"
				"<ul>\n";

			if (dir_path != "/") body += "<li><a href=\"../\">Parent Directory</a></li>\n";						// Enlace al directorio padre si no estamos en la raíz

			for (std::vector<std::string>::iterator it = directories.begin(); it != directories.end(); ++it)	// Añadir directorios
				body += "<li><a href=\"" + *it + "\">" + *it + "</a></li>\n";

			for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)				// Añadir archivos
				body += "<li><a href=\"" + *it + "\">" + *it + "</a></li>\n";

			body += "</ul>\n</body>\n</html>\n";

			std::string header =
				event->response_map["Protocol"] + " " + event->response_map["code"] + " " + event->response_map["code_description"] + "\r\n"
				"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
				"X-Content-Type-Options: nosniff\r\n"
				"Content-Length: " + Utils::ltos(body.size()) + "\r\n"
				"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

			event->response_map["header"] = header;
			event->response_map["body"] = body;
			event->file_info = 0;																	//	Set some flags
			event->file_read = 0;																	//	Set some flags
			event->file_size = 0;																	//	Set some flags
			event->write_buffer.clear();															//	Clear write_buffer
			event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());	//	Copy the header to write_buffer
	
			if (event->response_map["method"] != "HEAD") {
				event->response_size = body.size();
				event->file_size = header.size() + body.size();											//	Set the total size of the data to be sent
				event->write_buffer.insert(event->write_buffer.end(), body.begin(), body.end());		//	Copy the body to write_buffer
			}

			if (Epoll::set(event->fd, true, true) == -1) event->client->remove();					//	Set EPOLL to monitor write events

		}

	#pragma endregion

	#pragma region File

		void Protocol::response_file(EventInfo * event) {
			if (!event) return;

			std::string path = event->response_map["path"];
			if (path.empty()) { event->client->remove(); return; }

			if (event->header_map["Cache-Control"].empty()) {											//	If cache is allowed
				CacheInfo * fcache = Communication::cache.get(path);									//	Get the file from cache
				if (fcache) {																			//	If the file exist in cache
					std::string header =
						event->response_map["Protocol"] + " " + event->response_map["code"] + " " + event->response_map["code_description"] + "\r\n"
						"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
						"X-Content-Type-Options: nosniff\r\n"
						"Content-Length: " + Utils::ltos(fcache->content.size()) + "\r\n"
						"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

					event->file_info = 0;																					//	Set some flags
					event->file_read = 0;																					//	Set some flags
					event->file_size = 0;																					//	Set some flags
					event->write_buffer.clear();																			//	Clear write_buffer
					event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());					//	Copy the header to write_buffer

					if (event->response_map["method"] != "HEAD") {
						event->response_size = fcache->content.size();
						event->file_size = header.size() + fcache->content.size();												//	Set the total size of the data to be sent
						event->write_buffer.insert(event->write_buffer.end(), fcache->content.begin(), fcache->content.end());	//	Copy the body to write_buffer
					}

					if (Epoll::set(event->fd, true, true) == -1) event->client->remove();									//	Set EPOLL to monitor write events
					return;
				}
			}

			int fd = open(path.c_str(), O_RDONLY);														//	Open the file
			if (fd == -1) return;																		//	If error opening, return
			Utils::NonBlocking_FD(fd);																	//	Set the FD as non-blocking

			size_t filesize = Utils::filesize(fd);														//	Get the file size
			if (filesize == std::string::npos) { close(fd); event->client->remove(); return; }


			if (event->response_map["method"] == "HEAD") {
				close(fd);
				std::string header =
					event->response_map["Protocol"] + " " + event->response_map["code"] + " " + event->response_map["code_description"] + "\r\n"
					"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
					"X-Content-Type-Options: nosniff\r\n"
					"Content-Length: " + Utils::ltos(filesize) + "\r\n"
					"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

				event->file_info = 0;																		//	Set some flags
				event->file_read = 0;																		//	Set some flags
				event->file_size = 0;																		//	Set some flags
				event->write_buffer.clear();																//	Clear write_buffer
				event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());		//	Copy the header to write_buffer

				if (Epoll::set(event->fd, true, true) == -1) event->client->remove();						//	Set EPOLL to monitor write events
				return;
			}


			EventInfo event_data(fd, DATA, NULL, event->client);										//	Create the event for the DATA

			event_data.file_path = path;																//	Set the name of the file
			event_data.file_size = filesize;															//	Set the size of the file
			event_data.no_cache = !event->header_map["Cache-Control"].empty();							//	Set if the file must be added to cache

			if (pipe(event_data.pipe) == -1) { close(event_data.fd); return; event->client->remove(); }	//	Create the pipe for DATA (to be used with splice)
			Utils::NonBlocking_FD(event_data.pipe[0]);													//	Set the read end of the pipe as non-blocking
			Utils::NonBlocking_FD(event_data.pipe[1]);													//	Set the write end of the pipe as non-blocking

			size_t chunk = Communication::CHUNK_SIZE;																
			if (event_data.file_size > 0) chunk = std::min(event_data.file_size, Communication::CHUNK_SIZE);	//	Set the size of the chunk
			if (splice(event_data.fd, NULL, event_data.pipe[1], NULL, chunk, SPLICE_F_MOVE) == -1) {			//	Send the first chunk of data from the file to the pipe
				close(event_data.fd); close(event_data.pipe[0]); close(event_data.pipe[1]);						//	Splice failed, close FD's and return
				event->client->remove(); return;
			}
			std::swap(event_data.fd, event_data.pipe[0]);												//	Swap the read end of the pipe with the fd (this is to be consistent with the FD that EPOLL is monitoring)

			Event::events[event_data.fd] = event_data;													//	Add the DATA event to the event's list

			std::string header =
				event->response_map["Protocol"] + " " + event->response_map["code"] + " " + event->response_map["code_description"] + "\r\n"
				"Content-Type: " + event->response_map["Content-Type"] + "\r\n"
				"X-Content-Type-Options: nosniff\r\n"
				"Content-Length: " + Utils::ltos(event_data.file_size) + "\r\n"
				"Connection: " + event->response_map["Connection"] + "\r\n\r\n";

			event->file_info = 0;																		//	Set some flags
			event->file_read = 0;																		//	Set some flags
			event->file_size = header.size() + event_data.file_size;									//	Set the total size of the data to be sent
			event->response_size = event_data.file_size;
			event->write_buffer.clear();																//	Clear write_buffer
			event->write_buffer.insert(event->write_buffer.end(), header.begin(), header.end());		//	Copy the header to write_buffer

			if (Epoll::set(event->fd, !(event->header_map["Write_Only"] == "true"), true) == -1)		//	Set EPOLL to monitor write events for the client
				event->client->remove();
			else if (Epoll::add(event_data.fd, true, false) == -1) {									//	Set EPOLL to monitor read events for DATA
				event->file_size = 0;																	//	If set EPOLL fails, reset the flag,
				event->write_buffer.clear();															//	clear writte_buffer
				Event::remove(event_data.fd);															//	and remove the DATA event
				event->client->remove();
			}

			return;
		}

	#pragma endregion

	#pragma region CGI

		void Protocol::response_cgi(EventInfo * event) {
			(void) event;

			//	create fork and pipes
			//	create cgi_event
			//	if (content_length) {
				//	cgi_event->file_size == content_length;
			//	else
				//	cgi_event->file_info == 1;
		}


		//	?	CGI VARIABLES

		// Aquí tienes una lista de las variables de entorno más importantes que el servidor web configura y pasa al script CGI, junto con su explicación, en el contexto de una petición CGI típica.

		// Variables de entorno CGI

		// 1. REQUEST_METHOD:

		// Indica el método HTTP usado en la solicitud (por ejemplo, GET, POST, PUT, DELETE, etc.).

		// Ejemplo: REQUEST_METHOD=GET



		// 2. SCRIPT_NAME:

		// Contiene la ruta virtual del script CGI que está siendo ejecutado, tal como se solicitó en la URL.

		// Representa lo que el cliente solicitó en el navegador (por ejemplo, /cgi-bin/script.cgi).

		// Ejemplo: SCRIPT_NAME=/cgi-bin/script.cgi



		// 3. QUERY_STRING:

		// Contiene la parte de la URL después del ?, que incluye los parámetros enviados en la solicitud (GET).

		// Si no hay query string, estará vacía.

		// Ejemplo: QUERY_STRING=name=value&foo=bar



		// 4. PATH_INFO:

		// Contiene cualquier información adicional después del nombre del script en la URL.

		// Esto es útil si el script CGI recibe parámetros adicionales en la propia ruta (por ejemplo, /cgi-bin/script.cgi/extra/path).

		// Ejemplo: PATH_INFO=/extra/path



		// 5. PATH_TRANSLATED:

		// Contiene la ruta completa en el sistema de archivos del recurso solicitado. Se genera a partir de PATH_INFO y el directorio raíz del servidor.

		// Ejemplo: PATH_TRANSLATED=/var/www/cgi-bin/extra/path



		// 6. CONTENT_TYPE:

		// Indica el tipo de contenido de los datos que se están enviando en la solicitud, especialmente en métodos como POST.

		// Por ejemplo, si estás enviando datos de un formulario, el CONTENT_TYPE podría ser application/x-www-form-urlencoded.

		// Ejemplo: CONTENT_TYPE=application/x-www-form-urlencoded



		// 7. CONTENT_LENGTH:

		// Indica la longitud del cuerpo de la solicitud en bytes. Esto es relevante en solicitudes como POST donde se envían datos en el cuerpo.

		// Ejemplo: CONTENT_LENGTH=27



		// 8. SERVER_NAME:

		// Contiene el nombre del servidor o dominio donde se está ejecutando el CGI.

		// Ejemplo: SERVER_NAME=example.com



		// 9. SERVER_PORT:

		// Indica el número de puerto en el que está escuchando el servidor.

		// Ejemplo: SERVER_PORT=80



		// 10. SERVER_PROTOCOL:

		// Contiene la versión del protocolo HTTP que está utilizando el cliente.

		// Ejemplo: SERVER_PROTOCOL=HTTP/1.1



		// 11. REMOTE_ADDR:

		// Contiene la dirección IP del cliente que realizó la solicitud.

		// Ejemplo: REMOTE_ADDR=192.168.1.10



		// 12. REMOTE_PORT:

		// Indica el puerto desde el que el cliente ha hecho la solicitud.

		// Ejemplo: REMOTE_PORT=54321



		// 13. GATEWAY_INTERFACE:

		// Especifica la versión de CGI que está usando el servidor web.

		// Ejemplo: GATEWAY_INTERFACE=CGI/1.1



		// 14. HTTP_HOST:

		// Contiene el valor del encabezado Host enviado por el cliente. Es el nombre del host incluido en la solicitud.

		// Ejemplo: HTTP_HOST=example.com



		// 15. HTTP_USER_AGENT:

		// Contiene el valor del encabezado User-Agent que el cliente envía, describiendo el navegador o dispositivo que está haciendo la solicitud.

		// Ejemplo: HTTP_USER_AGENT=Mozilla/5.0 (Windows NT 10.0; Win64; x64)



		// 16. HTTP_ACCEPT:

		// Contiene los tipos de contenido que el cliente está dispuesto a aceptar, basado en la cabecera Accept.

		// Ejemplo: HTTP_ACCEPT=text/html,application/xhtml+xml



		// 17. HTTP_REFERER:

		// Indica la URL desde la que el cliente fue redirigido (si existe) mediante el encabezado Referer.

		// Ejemplo: HTTP_REFERER=http://example.com/previous_page.html



		// 18. REQUEST_URI (opcional, dependiendo del servidor web):

		// Contiene la ruta completa de la solicitud, tal como fue enviada por el cliente, incluyendo tanto el script como la query string.

		// Ejemplo: REQUEST_URI=/cgi-bin/script.cgi?name=value&foo=bar




		// Ejemplo completo de URL y cómo se reflejan las variables

		// Si la URL solicitada es:

		// http://example.com/cgi-bin/script.cgi/extra/path?name=value

		// Las variables de entorno CGI serán algo así:

		// REQUEST_METHOD=GET

		// SCRIPT_NAME=/cgi-bin/script.cgi

		// QUERY_STRING=name=value

		// PATH_INFO=/extra/path

		// PATH_TRANSLATED=/var/www/cgi-bin/extra/path

		// CONTENT_TYPE (vacío, ya que no hay cuerpo en un GET)

		// CONTENT_LENGTH (vacío, por la misma razón)

		// SERVER_NAME=example.com

		// SERVER_PORT=80

		// SERVER_PROTOCOL=HTTP/1.1

		// REMOTE_ADDR=192.168.1.10

		// REMOTE_PORT=54321

		// GATEWAY_INTERFACE=CGI/1.1


		// HTTP_HOST=example.com

		// HTTP_USER_AGENT=Mozilla/5.0 (Windows NT 10.0; Win64; x64)

		// HTTP_ACCEPT=text/html,application/xhtml+xml

		// HTTP_REFERER=http://example.com/previous_page.html

		// REQUEST_URI=/cgi-bin/script.cgi/extra/path?name=value


		// Resumen:

		// Estas variables de entorno proporcionan al script CGI toda la información necesaria sobre la solicitud, como el método HTTP, la URL solicitada, la query string, el tipo de contenido, la longitud del contenido, y los encabezados HTTP enviados por el cliente. El servidor web configura estas variables automáticamente antes de ejecutar el CGI.


		// Si hay un cuerpo en la solicitud (body):

		// El cuerpo de la solicitud (como los datos de un formulario enviado con POST) se envía al script CGI a través de la entrada estándar (stdin). El script CGI lo leerá de stdin si necesita procesar los datos.


		// Por ejemplo:

		// Si envías un formulario con el método POST, los datos del formulario se pasarán al CGI a través de stdin.

		// La variable de entorno CONTENT_LENGTH te dirá cuántos bytes están en el cuerpo para que el CGI sepa cuánto leer de stdin.

		// La variable CONTENT_TYPE indicará el tipo de los datos enviados (como application/x-www-form-urlencoded o multipart/form-data).


		// ¿Y el script CGI como argumento?

		// Sí, cuando ejecutas un CGI que es un script (como un archivo Python o Perl), si usas un intérprete (como python o perl), el nombre del script se pasa como un argumento al binario del intérprete, como en este ejemplo:

		// execve("/usr/bin/python", ["python", "/path/to/script.cgi"], envp);

		// Resumen del flujo:

		// 1. El cuerpo de la solicitud se pasa por stdin al CGI.


		// 2. El nombre del script CGI se pasa como argumento si se está utilizando un intérprete.

		// Ejemplo: execve("/usr/bin/python", ["python", "/path/to/script.cgi"], envp);



		// 3. Las variables de entorno (CONTENT_LENGTH, CONTENT_TYPE, etc.) indican la longitud y el tipo de los datos que el CGI debe leer de stdin.



		// De esta forma, los parámetros en la URL se pasan a través de variables de entorno como QUERY_STRING, mientras que los datos del cuerpo de la solicitud se pasan por stdin.

	#pragma endregion

#pragma endregion

#pragma region Process

	void Protocol::process_response(EventInfo * event) {
		if (!event) return;

		if 		(event->method == "Error")		response_error(event);
		else if (event->method == "Redirect")	response_redirect(event);
		else if (event->method == "Directory")	response_directory(event);
		else if (event->method == "File")		response_file(event);
		else if (event->method == "CGI")		response_cgi(event);
		else {
			//	Set a diferent error
			response_error(event);
		}
	}

#pragma endregion
