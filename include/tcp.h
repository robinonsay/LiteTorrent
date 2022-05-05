#ifndef TCP_H
#define TCP_H

#include "ltdefs.h"
#include <atomic>
#include <sys/socket.h>

class TCPServer{
    private:
        int sockfd;
        sockaddr_in addr;
    public:
        TCPServer(uint32_t port, bool blocking=true);
        int listen(uint32_t backlog);
        int accept(sockaddr_in *client_addr, size_t *addrlen);
        static int read(int cliSockfd, char *buff, size_t size, bool readAll=true);
        static int write(int cliSockfd, char *buff, size_t size, bool writeAll=true);
        static int close(int sockfd);
        static int shutdown(int sockfd);
        int getFD();
};

class TCPClient{
    private:
        int sockfd;
        sockaddr_in addr;
        std::atomic<bool> connected;
    public:
        TCPClient(const char ip[], uint32_t port, bool blocking=true);
        int connect();
        int read(char *buff, size_t size, bool readAll=true);
        int write(char *buff, size_t size, bool writeAll=true);
        int close();
        int getFD();
        bool isConn();
};

#endif

