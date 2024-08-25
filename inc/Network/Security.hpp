/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Security.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/25 11:19:12 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/25 18:56:34 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sstream>																						//	For std::stringstream to format strings
#include <iomanip>																						//	For stream manipulators like std::setw and std::setfill
#include <vector>

class Security {

	public:

		static std::string	encode_url(const std::string & url);
		static std::string	decode_url(const std::string & url);

		static std::string	encode_html(const std::string & content);
		static std::string	decode_html(const std::string & content);

		static std::string	encode_css(const std::string & content);
		static std::string	decode_css(const std::string & content);

		static std::string	encode_xml(const std::string & content);
		static std::string	decode_xml(const std::string & content);

		static std::string	encode_json(const std::string & content);
		static std::string	decode_json(const std::string & content);

		static std::string	encode_javascript(const std::string & content);
		static std::string	decode_javascript(const std::string & content);
		
};
