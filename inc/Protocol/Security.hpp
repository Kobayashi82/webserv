/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Security.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/25 11:19:12 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/26 21:58:53 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sstream>																						//	For std::stringstream to format strings
#include <iomanip>																						//	For stream manipulators like std::setw and std::setfill

#pragma region Security

	class Security {

		public:

			static std::string		KEY;																//	Key used to encrypt and decrypt data (used for cookies)

			static std::string	cipher_data(const std::string & data, std::string key = KEY);			//	Encrypts or decrypts data using the specified key
			
			static std::string	encode_url(const std::string & url);									//	Encodes a URL to make it safe for transmission over HTTP
			static std::string	decode_url(const std::string & url);									//	Decodes a URL to retrieve the original data from its encoded form

			static std::string	encode_html(const std::string & content);								//	Encodes HTML content to prevent issues with special characters and ensure correct display
			static std::string	decode_html(const std::string & content);								//	Decodes HTML content to retrieve the original text from its encoded form

			static std::string	encode_css(const std::string & content);								//	Encodes CSS content to escape special characters and ensure correct parsing
			static std::string	decode_css(const std::string & content);								//	Decodes CSS content to retrieve the original text from its encoded form

			static std::string	encode_xml(const std::string & content);								//	Encodes XML content to escape special characters and ensure correct parsing
			static std::string	decode_xml(const std::string & content);								//	Decodes XML content to retrieve the original text from its encoded form

			static std::string	encode_json(const std::string & content);								//	Encodes JSON content to escape special characters and ensure correct parsing
			static std::string	decode_json(const std::string & content);								//	Decodes JSON content to retrieve the original text from its encoded form

			static std::string	encode_javascript(const std::string & content);							//	Encodes JavaScript content to escape special characters and ensure correct parsing
			static std::string	decode_javascript(const std::string & content);							//	Decodes JavaScript content to retrieve the original text from its encoded form
			
	};

#pragma endregion
