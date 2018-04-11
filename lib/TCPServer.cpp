#include "TCPServer.h"

int TCPServer::parent_socket = -1;
int TCPServer::child_socket = -1;

TCPServer::TCPServer() {
    //Create parent socket
    parent_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (parent_socket < 0) {
        perror("Could not create socket");
        return;
    }

    //This trick is for reusing the last-used port immediately
    int optval = 1;
    setsockopt(parent_socket, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));

    //Prepare the server address structure
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(6685);

    //Bind the parent socket to all interface
    if (bind(parent_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Could not bind socket");
        return;
    }

    ////Let server listen for one connection
    if (listen(parent_socket, 5) < 0) {
        perror("Could not set on listening");
        return;
    }
}

int TCPServer::wait_for_connection() {
    int rc;
    int data = 0;

    rc = pthread_create(&serverThread, NULL, &wait_accept, 0);
    if (rc) {
        printf("Error:unable to create thread\n");
        exit(-1);
    }

    return rc;
}

std::string TCPServer::get_server_port() {
//    socklen_t len = sizeof(server_address);
//
//    if (getsockname(child_socket, (struct sockaddr *) &server_address, &len) == -1) {
//        return std::string();
//    }

    return std::to_string(ntohs(server_address.sin_port) / 256) + "," +
           std::to_string(ntohs(server_address.sin_port) % 256);
}

TCPServer::~TCPServer() {
    close(parent_socket);
}

void TCPServer::close_connection() {
    close(child_socket);
}

int TCPServer::Send(const char *buffer, int buffer_length) {
    return write(child_socket, buffer, buffer_length);
}

bool TCPServer::Send(const std::string &msg) {
    return write(child_socket, msg.c_str(), msg.size()) > 0;
}

int TCPServer::Receive(char *buffer, int buffer_length) {
    return read(child_socket, buffer, buffer_length);
}

std::string TCPServer::Receive(int nbytes) {
    char buffer[nbytes];

    if (read(child_socket, buffer, nbytes) < 0)
        return std::string();
    else return std::string(buffer);
}

void *TCPServer::wait_accept(void *) {
    socklen_t add_size;
    struct sockaddr_in client_address;

    pthread_detach(pthread_self());
    child_socket = accept(parent_socket, (struct sockaddr *) &client_address, &add_size);
}