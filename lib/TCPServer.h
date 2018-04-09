#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>

#define BUFSIZE 1024

class TCPServer
{
private:
    int parent_socket, child_socket;
    struct sockaddr_in server_address, client_address;

public:
    TCPServer();

    int wait_for_connection();

    std::string get_server_port();

    void close_connection();

    int Send(const char *buffer, int buffer_length);

    bool Send(const std::string &msg);

    int Receive(char *buffer, int buffer_length);

    std::string Receive(int nbytes = BUFSIZE);

    ~TCPServer();
};

#endif