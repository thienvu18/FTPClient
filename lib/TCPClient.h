#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string>

#ifndef BUFSIZE
#define BUFSIZE 1024
#endif

class TCPClient
{
private:
    int sock;

public:
    TCPClient();

    bool setup(std::string address, int port);

    bool isConnected();

    std::string get_client_address();

    int Send(const char *buffer, int buffer_length);

    bool Send(const std::string &msg);

    int Receive(char *buffer, int buffer_length);

    std::string Receive(int nbytes = BUFSIZE);

    void close_connection();

    ~TCPClient();
};

#endif
