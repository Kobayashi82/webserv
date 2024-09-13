/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Intermediary.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 12:49:18 by vzurera-          #+#    #+#             */
/*   Updated: 2024/09/13 15:25:02 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Protocol.hpp"
#include "Settings.hpp"

std::map<std::string, std::string> Intermediary::response_data(Net::EventInfo * event) {
	if (!event)	throw std::runtime_error("Event is null");
	std::map<std::string, std::string>	data;

	return (data);
}



// void popo(EventInfo * event) {
// 	if (!event) return;
// 	try {
// 		std::map<std::string, std::string>	data = Intermediary::response_data(event);
// 	} catch (...) {
// 		return;
// 	}
// }
