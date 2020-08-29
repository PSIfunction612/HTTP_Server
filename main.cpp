#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>

const int BACKLOG = 10;

int main(int argc, char *argv[])
{
    std::stringstream response;

    addrinfo hints, *servinfo;
    sockaddr_storage *their_addr;
    socklen_t addrlen;

    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, argv[1], &hints, &servinfo) != 0))
    {
        std::cerr << "error getaddrinfo()" << std::endl;
        return 1;
    }

    int sock_listen = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sock_listen < 0)
    {
        std::cerr << "error socket()" << std::endl;
        freeaddrinfo(servinfo);
        return 1;
    }

    if (bind(sock_listen, (sockaddr *)(servinfo->ai_addr), servinfo->ai_addrlen) < 0)
    {
        std::cerr << "error bind" << std::endl;
        close(sock_listen);
        freeaddrinfo(servinfo);
    }

    int on = 1;
    setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);

    if (listen(sock_listen, BACKLOG))
    {
        std::cerr << "error listen" << std::endl;
        close(sock_listen);
        freeaddrinfo(servinfo);
        return 1;
    }

    int sock;

    std::cout << "Ready to GO!!" << std::endl;

    // --------------------------------------------------

    while (true)
    {
        if ((sock = accept(sock_listen, (sockaddr *)&their_addr, &addrlen)) <= 0)
        {
            std::cerr << "error accepting" << std::endl;
            close(sock_listen);
            return 1;
        }

        std::ifstream index;
        index.open("./res/index.html", std::ios::binary);

        index.seekg(0, std::ios::end);
        int length = index.tellg();
        index.seekg(0, std::ios::beg);

        char *buffer = new char[length];
        index.read(buffer, length);
        index.close();

        std::string response_body(buffer);

        response << "HTTP/1.1 200 OK\r\n"
                 << "Version: HTTP/1.1\r\n"
                 << "Content-Type: text/html; charset=utf-8\r\n"
                 << "Content-Length: " << response_body.length() << "\r\n\r\n"
                 << response_body;

        if (send(sock, response.str().c_str(), response.str().length(), 0) < 0)
        {
            std::cerr << "Failed to send message" << std::endl;
        }
    }

    close(sock_listen);
    close(sock);
    freeaddrinfo(servinfo);

    return 0;
}