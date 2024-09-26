/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Security.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/25 11:19:15 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/26 20:27:57 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Security.hpp"

#pragma region Variables

	std::string		Security::KEY = "This_Key_Is_Super_Secret_And_Even_42_Hackers_Will_Never_Find_It!";	// Key used to encrypt and decrypt data (used for cookies)

#pragma endregion

#pragma region Cipher

	std::string Security::cipher_data(const std::string & data, std::string key) {
		std::string result = data; size_t key_length = key.size();

		for (size_t i = 0; i < data.size(); i++)
			result[i] = data[i] ^ key[i % key_length];

		return (result);
	}

#pragma endregion

#pragma region Encoder / Decoder

	#pragma region URL

		//	When sending or receiving a URL (ex. in a GET request)

		#pragma region Encode

			std::string Security::encode_url(const std::string & url) {
				std::ostringstream encode;
				encode << std::hex << std::uppercase;

				for (size_t i = 0; i < url.size(); ++i) {
					char ch = url[i];
					if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~' || ch == '/' || ch == ':' || ch == '?' || ch == '&' || ch == '=' || ch == '#' || ch == '%' || ch == '+') {
						encode << ch;
					} else {
						encode << '%' << std::setfill('0') << std::setw(2) << int((unsigned char)ch);
					}
				}

				return (encode.str());
			}

		#pragma endregion

		#pragma region Decode

			std::string Security::decode_url(const std::string & url) {
				std::ostringstream decode;

				for (size_t i = 0; i < url.size(); ++i) {
					if (url[i] == '%' && i + 2 < url.size()) {
						if (!isxdigit(url[i + 1]) || !isxdigit(url[i + 2]))	throw std::runtime_error("Invalid URL encoding");
						std::istringstream hex(url.substr(i + 1, 2));
						int value; if (hex >> std::hex >> value) {	decode << static_cast<char>(value); i += 2; }
					} else if (url[i] == '+')						decode << ' ';
					else											decode << url[i];
				}

				return (decode.str());
			}

		#pragma endregion

	#pragma endregion

	#pragma region HTML

	//	When sending HTML and receiving a Content-Type of text/html

		#pragma region Encode

			std::string Security::encode_html(const std::string & content) {
				std::string encode;
				encode.reserve(content.size());

				for (size_t i = 0; i < content.size(); ++i) {
					char ch = content[i];
					switch (ch) {
						case '&':	encode.append("&amp;");	 break;
						case '<':	encode.append("&lt;");	 break;
						case '>':	encode.append("&gt;");	 break;
						case '"':	encode.append("&quot;"); break;
						case '\'':	encode.append("&#39;");  break;
						default:	encode.push_back(ch);	 break;
					}
				}

				return (encode);
			}

		#pragma endregion

		#pragma region Decode

			std::string Security::decode_html(const std::string & content) {
				std::string decode;
				decode.reserve(content.size());

				for (size_t i = 0; i < content.size(); ++i) {
					if (content[i] == '&') {
						size_t sc_pos = content.find(';', i);
						if (sc_pos == std::string::npos)	throw std::runtime_error("Invalid HTML entity");
						std::string entity = content.substr(i, sc_pos - i + 1);

						if		(entity == "&amp;")			decode.push_back('&');
						else if	(entity == "&lt;")			decode.push_back('<');
						else if	(entity == "&gt;")			decode.push_back('>');
						else if	(entity == "&quot;")		decode.push_back('"');
						else if	(entity == "&#39;")			decode.push_back('\'');
						else 								throw std::runtime_error("Unknown HTML entity: " + entity);
						i = sc_pos;
					} else									decode.push_back(content[i]);
				}

				return (decode);
			}

		#pragma endregion

	#pragma endregion

	#pragma region CSS

		//	When sending CSS and receiving a Content-Type of text/css

		#pragma region Encode

			std::string Security::encode_css(const std::string & content) {
				if (content.find("<script>") != std::string::npos)
					throw std::runtime_error("400 Bad Request - Potentially dangerous content");

				std::ostringstream encode;
				for (size_t i = 0; i < content.size(); ++i) {
        			char ch = content[i];
					switch (ch) {
						case '&':	encode << "&amp;";	break;
						case '<':	encode << "&lt;";	break;
						case '>':	encode << "&gt;";	break;
						case '"':	encode << "&quot;";	break;
						case '\'':	encode << "&#39;";	break;
						case '\\':	encode << "\\\\";	break;
						default:	encode << ch;		break;
					}
				}
				return (encode.str());
			}

		#pragma endregion

		#pragma region Decode

			std::string Security::decode_css(const std::string & content) {
				if (content.find("<script>") != std::string::npos)
					throw std::runtime_error("400 Bad Request - Potentially dangerous content");

				std::string decode;
				decode.reserve(content.size());

				for (size_t i = 0; i < content.size(); ++i) {
					char ch = content[i];
					if (ch == '\\' && i + 1 < content.size()) {
						char next_ch = content[i + 1];
						switch (next_ch) {
							case '\\':	decode.push_back('\\'); ++i; break;
							case 'A':	decode.push_back('\n'); ++i; break;
							case '"':	decode.push_back('"');  ++i; break;
							default:	decode.push_back(ch);		 break;
						}
					} else				decode.push_back(ch);
				}

				return (decode);
			}

		#pragma endregion

	#pragma endregion

	#pragma region XML

		//	When sending XML and receiving a Content-Type of text/xml or application/xml

		#pragma region Encode

			std::string Security::encode_xml(const std::string & content) {
				std::ostringstream encode;

				for (size_t i = 0; i < content.size(); ++i) {
        			char ch = content[i];
					switch (ch) {
						case '&':	encode << "&amp;";	break;
						case '<':	encode << "&lt;";	break;
						case '>':	encode << "&gt;";	break;
						case '"':	encode << "&quot;";	break;
						case '\'':	encode << "&apos;";	break;
						default:	encode << ch;		break;
					}
				}
				return (encode.str());
			}

		#pragma endregion

		#pragma region Decode

			std::string Security::decode_xml(const std::string & content) {
				std::string decode;
				decode.reserve(content.size());

				for (size_t i = 0; i < content.size(); ++i) {
					if (content[i] == '&') {
						size_t sc_pos = content.find(';', i);
						if (sc_pos == std::string::npos)	throw std::runtime_error("Invalid XML entity");
						std::string entity = content.substr(i, sc_pos - i + 1);
						if		(entity == "&amp;")			decode.push_back('&');
						else if	(entity == "&lt;")			decode.push_back('<');
						else if	(entity == "&gt;")			decode.push_back('>');
						else if	(entity == "&quot;")		decode.push_back('"');
						else if	(entity == "&apos;")		decode.push_back('\'');
						else								throw std::runtime_error("Unknown XML entity: " + entity);
						i = sc_pos;
					} else									decode.push_back(content[i]);
				}

				return (decode);
			}

		#pragma endregion

	#pragma endregion

	#pragma region JSON

		//	When sending JSON and receiving a Content-Type of application/json

		#pragma region Encode

			std::string Security::encode_json(const std::string & content) {
				std::ostringstream encode;

				for (size_t i = 0; i < content.size(); ++i) {
    				char ch = content[i];
					switch (ch) {
						case '\\':	encode << "\\\\";	break;
						case '"':	encode << "\\\"";	break;
						case '\n':	encode << "\\n";	break;
						case '\r':	encode << "\\r";	break;
						case '\b':	encode << "\\b";	break;
						case '\f':	encode << "\\f";	break;
						case '\t':	encode << "\\t";	break;
						default:	encode << ch;		break;
					}
				}

				return (encode.str());
		}

		#pragma endregion

		#pragma region Decode

			std::string Security::decode_json(const std::string & content) {
				std::ostringstream decode;

				for (size_t i = 0; i < content.size(); ++i) {
					if (content[i] == '\\' && i + 1 < content.size()) {
						char next_ch = content[i + 1];
						switch (next_ch) {
							case '\\':	decode << '\\';	break;
							case '"':	decode << '"';	break;
							case 'n':	decode << '\n';	break;
							case 'r':	decode << '\r';	break;
							case 'b':	decode << '\b';	break;
							case 'f':	decode << '\f';	break;
							case 't':	decode << '\t';	break;
							case 'u': {
								if (i + 5 >= content.size())								throw std::runtime_error("Invalid Unicode escape in JSON");
								std::string hex_code = content.substr(i + 2, 4);
								int code;
								if (!(std::istringstream(hex_code) >> std::hex >> code))	throw std::runtime_error("Invalid Unicode escape in JSON");
								decode << static_cast<char>(code); i += 4;	break;
							}
							default:	decode << next_ch;					break;
						} i++;
					} else				decode << content[i];
				}

				return (decode.str());
			}

		#pragma endregion

	#pragma endregion

	#pragma region JavaScript

		//	When sending JavaScript, if the Content-Type is application/javascript or text/javascript

		#pragma region Encode

			std::string Security::encode_javascript(const std::string & content) {
				std::ostringstream encode;

				 for (size_t i = 0; i < content.size(); ++i) {
        			char ch = content[i];
					switch (ch) {
						case '\\':	encode << "\\\\";	break;
						case '"':	encode << "\\\"";	break;
						case '\'':	encode << "\\'";	break;
						case '\n':	encode << "\\n";	break;
						case '\r':	encode << "\\r";	break;
						default:	encode << ch;		break;
					}
				}

				return (encode.str());
			}

		#pragma endregion

		#pragma region Decode

			std::string Security::decode_javascript(const std::string & content) {
				std::ostringstream decode;

				for (size_t i = 0; i < content.size(); ++i) {
					if (content[i] == '\\' && i + 1 < content.size()) {
						char next_ch = content[i + 1];
						switch (next_ch) {
							case '\\':	decode << '\\';		break;
							case '"':	decode << '"';		break;
							case '\'':	decode << '\'';		break;
							case 'n':	decode << '\n';		break;
							case 'r':	decode << '\r';		break;
							default:	decode << next_ch;	break;
						} i++;
					} else				decode << content[i];
				}

				return (decode.str());
			}

		#pragma endregion

	#pragma endregion

#pragma endregion
