/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Codes.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 14:02:09 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/08 15:12:54 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Settings.hpp"

void load_error_codes() {
    Settings::error_codes[100] = "Continue";
    Settings::error_codes[101] = "Switching Protocols";
    Settings::error_codes[102] = "Processing";
    Settings::error_codes[200] = "OK";
    Settings::error_codes[201] = "Created";
    Settings::error_codes[202] = "Accepted";
    Settings::error_codes[203] = "Non-Authoritative Information";
    Settings::error_codes[204] = "No Content";
    Settings::error_codes[205] = "Reset Content";
    Settings::error_codes[206] = "Partial Content";
    Settings::error_codes[207] = "Multi-Status";
    Settings::error_codes[208] = "Already Reported";
    Settings::error_codes[226] = "IM Used";
    Settings::error_codes[300] = "Multiple Choices";
    Settings::error_codes[301] = "Moved Permanently";
    Settings::error_codes[302] = "Found";
    Settings::error_codes[303] = "See Other";
    Settings::error_codes[304] = "Not Modified";
    Settings::error_codes[305] = "Use Proxy";
    Settings::error_codes[306] = "(Unused)";
    Settings::error_codes[307] = "Temporary Redirect";
    Settings::error_codes[308] = "Permanent Redirect";
    Settings::error_codes[400] = "Bad Request";
    Settings::error_codes[401] = "Unauthorized";
    Settings::error_codes[402] = "Payment Required";
    Settings::error_codes[403] = "Forbidden";
    Settings::error_codes[404] = "Not Found";
    Settings::error_codes[405] = "Method Not Allowed";
    Settings::error_codes[406] = "Not Acceptable";
    Settings::error_codes[407] = "Proxy Authentication Required";
    Settings::error_codes[408] = "Request Timeout";
    Settings::error_codes[409] = "Conflict";
    Settings::error_codes[410] = "Gone";
    Settings::error_codes[411] = "Length Required";
    Settings::error_codes[412] = "Precondition Failed";
    Settings::error_codes[413] = "Payload Too Large";
    Settings::error_codes[414] = "URI Too Long";
    Settings::error_codes[415] = "Unsupported Media Type";
    Settings::error_codes[416] = "Range Not Satisfiable";
    Settings::error_codes[417] = "Expectation Failed";
    Settings::error_codes[418] = "I'm a teapot";
    Settings::error_codes[421] = "Misdirected Request";
    Settings::error_codes[422] = "Unprocessable Entity";
    Settings::error_codes[423] = "Locked";
    Settings::error_codes[424] = "Failed Dependency";
    Settings::error_codes[425] = "Too Early";
    Settings::error_codes[426] = "Upgrade Required";
    Settings::error_codes[428] = "Precondition Required";
    Settings::error_codes[429] = "Too Many Requests";
    Settings::error_codes[431] = "Request Header Fields Too Large";
    Settings::error_codes[451] = "Unavailable For Legal Reasons";
    Settings::error_codes[500] = "Internal Server Error";
    Settings::error_codes[501] = "Not Implemented";
    Settings::error_codes[502] = "Bad Gateway";
    Settings::error_codes[503] = "Service Unavailable";
    Settings::error_codes[504] = "Gateway Timeout";
    Settings::error_codes[505] = "HTTP Version Not Supported";
}
