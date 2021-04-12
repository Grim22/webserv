/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: julnolle <julnolle@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/16 15:55:26 by julnolle          #+#    #+#             */
/*   Updated: 2021/04/12 19:24:51 by julnolle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfParser.hpp"
#include "HttpBlock.hpp"

int main()
{
	ConfParser parser("nginx.conf");

	try {
		parser.readConfFile();


		std::cout << std::endl << "EXTRACT CONFIG AND GET ALL SERVER BLOCKS: " << std::endl;

		HttpBlock config = parser.getHttpBlock();

		std::vector<ServerBlock> servers = config.getServers();
		std::cout << "Number of servers: " << servers.size() << std::endl;


		displayVec(servers, '\n');
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
	}

	// servers.getbyIP(ip, port, host);

	return 0;
}