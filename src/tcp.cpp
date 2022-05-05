#include "tcp.h"
#include "errors.h"
#include "ltdefs.h"

#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

TCPServer::TCPServer(uint32_t port, bool blocking){
    int status;
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(this->sockfd < 0) sysError("ERROR opening socket");
    if(!blocking) fcntl(this->sockfd, F_SETFL, O_NONBLOCK);
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(port);
    this->addr.sin_addr.s_addr = INADDR_ANY;
    status = bind(this->sockfd,
                  (struct sockaddr *) &this->addr,
                  sizeof(this->addr));
    if(status < 0) sysError("ERROR binding socket");
}
int TCPServer::listen(uint32_t backlog){
    return ::listen(this->sockfd, backlog);
}
int TCPServer::accept(sockaddr_in *client_addr, size_t *addrlen){
    return ::accept(this->sockfd, (struct sockaddr *) client_addr, (socklen_t *) addrlen);
}
int TCPServer::read(int cliSockfd, char *buff, size_t size, bool readAll){
    return ::read(cliSockfd, buff, size);
}
int TCPServer::write(int cliSockfd, char *buff, size_t size, bool writeAll){
    return ::write(cliSockfd, buff, size);
}
int TCPServer::close(int sockfd){
    return ::close(sockfd);
}

int TCPServer::shutdown(int sockfd){
    int status;
    char buff[1];
    do{
        status = ::read(sockfd, buff, sizeof(buff));
        if(status < 0) return status;
    }while (status != 0);
    return ::close(sockfd);
}

int TCPServer::getFD(){
    return this->sockfd;
}

TCPClient::TCPClient(const char ip[], uint32_t port, bool blocking): connected(false){
    int status;
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(this->sockfd < 0) sysError("ERROR opening socket");
    if(!blocking) fcntl(this->sockfd, F_SETFL, O_NONBLOCK);
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(port);
    status = inet_aton(ip, &this->addr.sin_addr);
    if(status < 0) fatalError(std::cerr, "Error: Invalid IPv4 address");
}
int TCPClient::connect(){
    int status;
    status = ::connect(this->sockfd, (sockaddr *) &this->addr, sizeof(this->addr));
    this->connected = status >= 0;
    return status;
}
int TCPClient::read(char *buff, size_t size, bool readAll){
    return ::read(this->sockfd, buff, size);
}
int TCPClient::write(char *buff, size_t size, bool writeAll){
    return ::write(this->sockfd, buff, size);
}
int TCPClient::close(){
    return ::close(this->sockfd);
}
int TCPClient::getFD(){
    return this->sockfd;
}

bool TCPClient::isConn(){
    return this->connected;
}

