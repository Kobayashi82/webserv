/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test-cgi.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/29 17:23:54 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/30 14:49:01 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>
#include <map>

//	Función para escapar caracteres HTML
//	(usado para enviar el contenido del cuerpo que hemos recibido)
//	No es importante y no se usará para nada
std::string escapeHtml(const std::string & input, char * isShell) {
    std::string output;

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];

        switch (c) {
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '&': output += "&amp;"; break;
            case '"': output += "&quot;"; break;
			case '\n':
				if (!isShell)	output += "<br>";
				else			output += c;
				break;
            default: output += c; break;
        }
    }

    return (output);
}

int main(int argc, char **argv) {
	std::string input;
    std::map<std::string, std::pair<std::string, std::string> > variables;

	if (std::getenv("SHELL") || argc > 1 && std::string(argv[1]) == "test") {
		input =
			"\n--boundary123\n"
			"Content-Disposition: form-data; name=\"username\"\n"
			"\n"
			"johndoe\n"
			"--boundary123\n"
			"Content-Disposition: form-data; name=\"password\"\n"
			"\n"
			"securepassword123\n"
			"--boundary123\n"
			"Content-Disposition: form-data; name=\"email\"\n"
			"\n"
			"johndoe@example.com\n"
			"--boundary123\n"
			"Content-Disposition: form-data; name=\"age\"\n"
			"\n"
			"30\n"
			"--boundary123\n"
			"Content-Disposition: form-data; name=\"profile_picture\"; filename=\"profile.jpg\"\n"
			"Content-Type: image/jpeg\n"
			"\n"
			"<contenido binario del archivo aquí>\n"
			"--boundary123--\n";

		variables["CONTENT_LENGTH"]		=	std::make_pair("CONTENT_LENGTH",		"Indica la longitud del cuerpo de la solicitud en bytes. Puede estar vacío");
		variables["CONTENT_TYPE"]		=	std::make_pair("CONTENT_TYPE",			"Indica el tipo de contenido de los datos. Puede estar vacío");
		variables["GATEWAY_INTERFACE"]	=	std::make_pair("GATEWAY_INTERFACE",		"La versión de CGI que está usando el servidor web");
		variables["HTTPS"]				=	std::make_pair("HTTPS",					"Indica si la conexión es segura (HTTPS)");
		variables["HTTP_ACCEPT"]		=	std::make_pair("HTTP_ACCEPT",			"Los tipos de contenido que el cliente está dispuesto a aceptar");
		variables["HTTP_HOST"]			=	std::make_pair("HTTP_HOST",				"El nombre del servidor virtual que está manejando la solicitud");
		variables["HTTP_REFERER"]		=	std::make_pair("HTTP_REFERER",			"El valor de 'Referer', que indica la página anterior a la que se hizo la solicitud");
		variables["HTTP_USER_AGENT"]	=	std::make_pair("HTTP_USER_AGENT",		"El contenido del encabezado 'User-Agent', que identifica el navegador del cliente");
		variables["PATH_INFO"]			=	std::make_pair("PATH_INFO",				"Cualquier información adicional después del nombre del script en la URL");
		variables["PATH_TRANSLATED"]	=	std::make_pair("PATH_TRANSLATED",		"La ruta completa en el sistema de archivos del recurso solicitado");
		variables["QUERY_STRING"]		=	std::make_pair("QUERY_STRING",			"Es la cadena de consulta (query string), que contiene los parámetros enviados después de ?");
		variables["REDIRECT_STATUS"]	=	std::make_pair("REDIRECT_STATUS",		"Código de estado de la anterior redirección. Puede estar vacío");
		variables["REMOTE_ADDR"]		=	std::make_pair("REMOTE_ADDR",			"La dirección IP del cliente que hizo la solicitud");
		variables["REMOTE_PORT"]		=	std::make_pair("REMOTE_PORT",			"El puerto del cliente que hizo la solicitud");
		variables["REQUEST_FILENAME"]	=	std::make_pair("REQUEST_FILENAME",		"Ruta completa del archivo solicitado");
		variables["REQUEST_METHOD"]		=	std::make_pair("REQUEST_METHOD",		"El método HTTP utilizado en la solicitud (GET, POST, PUT, DELETE, etc.)");
		variables["REQUEST_SCHEME"]		=	std::make_pair("REQUEST_SCHEME",		"Esquema de la solicitud (HTTP o HTTPS)");
		variables["REQUEST_URI"]		=	std::make_pair("REQUEST_URI",			"Es la URI completa incluyendo la cadena de consulta (query string)");
		variables["SCRIPT_NAME"]		=	std::make_pair("SCRIPT_NAME",			"La ruta virtual del script CGI que está siendo ejecutado, tal como se solicitó en la URL");
		variables["SERVER_NAME"]		=	std::make_pair("SERVER_NAME",			"El valor del encabezado host, que es el nombre del dominio o la dirección IP solicitada");
		variables["SERVER_PORT"]		=	std::make_pair("SERVER_PORT",			"El puerto del servidor que está manejando la solicitud");
		variables["SERVER_PROTOCOL"]	=	std::make_pair("SERVER_PROTOCOL",		"El protocolo HTTP que está utilizando el cliente");
		variables["SERVER_SOFTWARE"]	=	std::make_pair("SERVER_SOFTWARE",		"Nombre y versión del software del servidor");
	} else {
		input = std::string((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());

		if (std::getenv("CONTENT_LENGTH"))		variables["CONTENT_LENGTH"]		=	std::make_pair(std::getenv("CONTENT_LENGTH"),		"Indica la longitud del cuerpo de la solicitud en bytes. Puede estar vacío");
		if (std::getenv("CONTENT_TYPE"))		variables["CONTENT_TYPE"]		=	std::make_pair(std::getenv("CONTENT_TYPE"),			"Indica el tipo de contenido de los datos. Puede estar vacío");
		if (std::getenv("GATEWAY_INTERFACE"))	variables["GATEWAY_INTERFACE"]	=	std::make_pair(std::getenv("GATEWAY_INTERFACE"),	"La versión de CGI que está usando el servidor web");
		if (std::getenv("HTTPS"))				variables["HTTPS"]				=	std::make_pair(std::getenv("HTTPS"),				"Indica si la conexión es segura (HTTPS)");
		if (std::getenv("HTTP_ACCEPT"))			variables["HTTP_ACCEPT"]		=	std::make_pair(std::getenv("HTTP_ACCEPT"),			"Los tipos de contenido que el cliente está dispuesto a aceptar");
		if (std::getenv("HTTP_HOST"))			variables["HTTP_HOST"]			=	std::make_pair(std::getenv("HTTP_HOST"),			"El nombre del servidor virtual que está manejando la solicitud");
		if (std::getenv("HTTP_REFERER"))		variables["HTTP_REFERER"]		=	std::make_pair(std::getenv("HTTP_REFERER"),			"El valor de 'Referer', que indica la página anterior a la que se hizo la solicitud");
		if (std::getenv("HTTP_USER_AGENT"))		variables["HTTP_USER_AGENT"]	=	std::make_pair(std::getenv("HTTP_USER_AGENT"),		"El contenido del encabezado 'User-Agent', que identifica el navegador del cliente");
		if (std::getenv("PATH_INFO"))			variables["PATH_INFO"]			=	std::make_pair(std::getenv("PATH_INFO"),			"Cualquier información adicional después del nombre del script en la URL");
		if (std::getenv("PATH_TRANSLATED"))		variables["PATH_TRANSLATED"]	=	std::make_pair(std::getenv("PATH_TRANSLATED"),		"La ruta completa en el sistema de archivos del recurso solicitado");
		if (std::getenv("QUERY_STRING"))		variables["QUERY_STRING"]		=	std::make_pair(std::getenv("QUERY_STRING"),			"Es la cadena de consulta (query string), que contiene los parámetros enviados después de ?");
		if (std::getenv("REDIRECT_STATUS"))		variables["REDIRECT_STATUS"]	=	std::make_pair(std::getenv("REDIRECT_STATUS"),		"Código de estado de la anterior redirección. Puede estar vacío");
		if (std::getenv("REMOTE_ADDR"))			variables["REMOTE_ADDR"]		=	std::make_pair(std::getenv("REMOTE_ADDR"),			"La dirección IP del cliente que hizo la solicitud");
		if (std::getenv("REMOTE_PORT"))			variables["REMOTE_PORT"]		=	std::make_pair(std::getenv("REMOTE_PORT"),			"El puerto del cliente que hizo la solicitud");
		if (std::getenv("REQUEST_FILENAME"))	variables["REQUEST_FILENAME"]	=	std::make_pair(std::getenv("REQUEST_FILENAME"),		"Ruta completa del archivo solicitado");
		if (std::getenv("REQUEST_METHOD"))		variables["REQUEST_METHOD"]		=	std::make_pair(std::getenv("REQUEST_METHOD"),		"El método HTTP utilizado en la solicitud (GET, POST, PUT, DELETE, etc.)");
		if (std::getenv("REQUEST_SCHEME"))		variables["REQUEST_SCHEME"]		=	std::make_pair(std::getenv("REQUEST_SCHEME"),		"Esquema de la solicitud (HTTP o HTTPS)");
		if (std::getenv("REQUEST_URI"))			variables["REQUEST_URI"]		=	std::make_pair(std::getenv("REQUEST_URI"),			"Es la URI completa incluyendo la cadena de consulta (query string)");
		if (std::getenv("SCRIPT_NAME"))			variables["SCRIPT_NAME"]		=	std::make_pair(std::getenv("SCRIPT_NAME"),			"La ruta virtual del script CGI que está siendo ejecutado, tal como se solicitó en la URL");
		if (std::getenv("SERVER_NAME"))			variables["SERVER_NAME"]		=	std::make_pair(std::getenv("SERVER_NAME"),			"El valor del encabezado host, que es el nombre del dominio o la dirección IP solicitada");
		if (std::getenv("SERVER_PORT"))			variables["SERVER_PORT"]		=	std::make_pair(std::getenv("SERVER_PORT"),			"El puerto del servidor que está manejando la solicitud");
		if (std::getenv("SERVER_PROTOCOL"))		variables["SERVER_PROTOCOL"]	=	std::make_pair(std::getenv("SERVER_PROTOCOL"),		"El protocolo HTTP que está utilizando el cliente");
		if (std::getenv("SERVER_SOFTWARE"))		variables["SERVER_SOFTWARE"]	=	std::make_pair(std::getenv("SERVER_SOFTWARE"),		"Nombre y versión del software del servidor");

		if (input.empty() && variables.empty()) { std::cout << "HTTP/1.1 404 Not Found\r\n\r\n"; return (0); }
	}

	std::string body =
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"    <meta charset=\"UTF-8\">\n"
		"    <title>Test-CGI</title>\n"
		"    <style>\n"
		"        body {\n"
		"            font-family: Arial, sans-serif;\n"
		"            background-color: #f4f4f4;\n"
		"            margin: 0;\n"
		"            padding: 20px;\n"
		"        }\n"
		"        h1 {\n"
		"            color: #333;\n"
		"			text-align: center;\n"
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
		"        .Nombre {\n"
		"            min-width: 200px;\n"
	    "            max-width: 200px;\n"
		"            text-align: left;\n"
		"            word-wrap: break-word;\n"
		"        }\n"
		"        .Valor {\n"
  		"            min-width: 300px;\n"
	    "            max-width: 400px;\n"
		"            text-align: left;\n"
		"            word-wrap: break-word;\n"
		"        }\n"
		"        .Descripcion {\n"
		"            min-width: 400px;\n"
		"            text-align: left;\n"
		"            word-wrap: break-word;\n"
		"        }\n"
		"    </style>\n"
		"</head>\n"
		"<body>\n";

	if (variables.size() > 0) {
		body +=
		"    <h1>Variables</h1>\n"
		"    <table>\n"
		"        <thead>\n"
		"            <tr>\n"
		"                <th>Nombre</th>\n"
		"                <th class=\"Valor\">Valor</th>\n"
		"                <th class=\"Descripcion\">Descripción</th>\n"
		"            </tr>\n"
		"        </thead>\n"
		"        <tbody>\n";

		for (std::map<std::string, std::pair<std::string, std::string> >::iterator it = variables.begin(); it != variables.end(); ++it) {
			body +=
				"            <tr>\n"
				"                <td class=\"Nombre\"><strong>" + it->first + "</strong></td>\n"
				"                <td class=\"Valor\" style=\"color: #333;\">" + it->second.first + "</td>\n"
				"                <td class=\"Descripcion\">" + it->second.second + "</td>\n"
				"            </tr>\n";
		}
	}

	body +=
		"        </tbody>\n"
		"    </table>\n";

	if (!input.empty()) body +=
		"	<h1>Cuerpo de la solicitud</h1>\n"
		"	<p>" + escapeHtml(input, std::getenv("SHELL")) + "</p>\n";

	body +=
		"</body>\n"
		"</html>\n";

	std::stringstream header;
    header	<< "Status: 200 OK\r\n"
			<< "Content-Type: text/html\r\n"
			<< "Content-Length: " << body.length() << "\r\n"
			<< "\r\n";

	std::cout << header.str() << body;
}
