#ifndef WEBSERV_HPP
#define WEBSERV_HPP


# include <string>
# include <iostream>

# include <sys/socket.h> // For socket functions
# include <netinet/in.h> // For sockaddr_in
# include <cstdlib> // For exit() and EXIT_FAILURE an atoi(char*)
# include <fstream> // ifstream
# include <fcntl.h> // For fcntl --> non_blocking fd
# include <map>

# include <sstream>
# include <bitset>


// Default server config values
# define DEFAULT_INDEX			"index.html"
# define DEFAULT_AUTOINDEX		false
# define DEFAULT_MAX_BDY_SIZE	1000000			// 1MB
# define DEFAULT_KEEPALIVE_T	75				// seconds
# define DEFAULT_LISTEN_IP		"0.0.0.0"
# define DEFAULT_LISTEN_PORT	80
# define DEFAULT_SRV_NAME		""
# define DEFAULT_ROOT			"html"
# define DEFAULT_AUTH_BASIC		"off"
# define DEFAULT_CHUNKED_ENC	true


// Colors
# define RED		"\033[31;1m"
# define GREEN		"\033[32;1m"
# define YELLOW		"\033[33;1m"
# define WHITE		"\033[37;1m"
# define NOCOLOR	"\033[0m"

#endif