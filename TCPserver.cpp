#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // For read
#include <fstream> // ifstream
#include <sstream> // sstream

int main() {
  // Create a socket (IPv4, TCP)
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    std::cout << "Failed to create socket. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  // Listen to port 9999 on any address
  sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(9999); // htons is necessary to convert a number to
                                   // network byte order
  if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
    std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }
// Start listening. Hold at most 10 connections in the queue
  if (listen(sockfd, 10) < 0) {
    std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  // Grab a connection from the queue
  auto addrlen = sizeof(sockaddr);
  int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
  if (connection < 0) {
    std::cout << "Failed to grab connection. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  // Read from the connection until an empty line is found. Concatenate the successive buffers into a stringstream
  char buffer[10];
  std::stringstream data;
  bool go_on = true;
  int len;
  while (go_on)
  {
    len = read(connection, buffer, 9);
    buffer[len] = 0;
    // std::cout << "buf: " << buffer << std::endl;
    // std::cout << "len: " << len << std::endl;
    data << buffer;
    go_on = data.str().find("\r\n\r\n") == std::string::npos;
  }
  std::cout << data.str() << std::endl;
  
  // Send a message to the connection
    std::ifstream file;
    file.open("nginx.conf");
    if (file.fail())
    {
        std::cout << "cant open file" << std::endl;
    }
    std::string line;
    while (getline(file, line))
    {
        send(connection, line.c_str(), line.size(), 0);
        send(connection, "\n", 1, 0);
    }
    
  std::string response = "Good talking to you\n";
  send(connection, response.c_str(), response.size(), 0);

  // Close the connections
  close(connection);
  close(sockfd);
}