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
    static int parent_socket;
    static int child_socket;
    struct sockaddr_in server_address;

    pthread_t serverThread;

    static void *wait_accept(void *);

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