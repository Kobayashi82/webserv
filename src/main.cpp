/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 14:30:55 by vzurera-          #+#    #+#             */
/*   Updated: 2024/07/29 21:13:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

//  *       service nginx reload

//  TODO    Parse mimo.types
//  TODO    Terminal interface

//  Entry point
int main(int argc, char **argv) {
    Settings::load_args(argc, argv); if (Settings::terminate != -1) return (Settings::terminate);

    enableRawMode();
    while (Settings::terminate == -1) {
        Output();
    } disableRawMode();

    Settings::clear();
    return (Settings::terminate);
}
