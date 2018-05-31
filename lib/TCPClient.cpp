
#include "TCPClient.h"

TCPClient::TCPClient() {
    sock = -1;
}

bool TCPClient::setup(std::string address, int port) {
    struct sockaddr_in server;

    if (sock < 0) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            printf("Could not create socket\n");
        }
    }
    int n = inet_addr(address.c_str());
    if (n < 0) {
        struct hostent *he;
        struct in_addr **addr_list;
        if ((he = gethostbyname(address.c_str())) == NULL) {
            printf("Failed to resolve hostname\n");
            sock = -1;
            return false;
        }
        addr_list = (struct in_addr **) he->h_addr_list;
        for (int i = 0; addr_list[i] != NULL; i++) {
            server.sin_addr = *addr_list[i];
            break;
        }
    } else {
        server.sin_addr.s_addr = inet_addr(address.c_str());
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        printf("connect failed. Error");
        sock - 1;
        return false;
    }
    return true;
}

int TCPClient::Send(const char *buffer, int buffer_length) {
    return write(sock, buffer, buffer_length);
}

bool TCPClient::Send(const std::string &msg) {
    return write(sock, msg.c_str(), msg.size()) >= 0;
}

int TCPClient::Receive(char *buffer, int buffer_length) {
    memset(buffer, 0, BUFSIZE);
    return read(sock, buffer, buffer_length);
}

std::string TCPClient::Receive(int nbytes) {
    char buffer[nbytes + 1];
    memset(buffer, 0, nbytes + 1);

    if (read(sock, buffer, nbytes) < 0)
        return std::string();
    else return std::string(buffer);
}

TCPClient::~TCPClient() {
    close(sock);
}

std::string TCPClient::get_client_address() {
    std::string addr;
    struct sockaddr_in client_address;
    socklen_t len = sizeof(client_address);

    if (getsockname(sock, (struct sockaddr *) &client_address, &len) == -1) {
        perror("getsockname");
        return std::string();
    }

    addr = inet_ntoa(client_address.sin_addr);
    for (int i = addr.size() - 1; i >= 0; i--) {
        if (addr[i] == '.') addr.replace(i, 1, ",");
    }

    return addr;
}

bool TCPClient::isConnected() {
    return sock > 0;
}

void TCPClient::close_connection() {
    close(sock);
}
