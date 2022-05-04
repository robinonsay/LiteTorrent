#ifndef TCP_H
#define TCP_H

#include "ltdefs.h"
#include <sys/socket.h>

class TCPServer{
    private:
        int sockfd;
        sockaddr_in addr;
    public:
        TCPServer(uint32_t port, bool blocking=true);
        int listen(uint32_t backlog);
        int accept(sockaddr_in *client_addr, size_t *addrlen);
        static int read(int cliSockfd, char *buff, size_t size, bool bestEffort=true);
        static int write(int cliSockfd, char *buff, size_t size, bool bestEffort=true);
        static int close(int sockfd);
        int getFD();
};

class TCPClient{
    private:
        int sockfd;
        sockaddr_in addr;
    public:
        TCPClient(const char ip[], uint32_t port, bool blocking=true);
        int connect();
        int read(char *buff, size_t size, bool bestEffort=true);
        int write(char *buff, size_t size, bool bestEffort=true);
        int close();
        int getFD();
};

#endif

