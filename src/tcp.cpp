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
    int bytesRead = 0;
    int pos = 0;
    if(!readAll){
        pos = ::read(cliSockfd, buff, size);
    }else{
        do{
            bytesRead = ::read(cliSockfd, &buff[pos], size - (uint32_t) pos);
            if(bytesRead < 0) return bytesRead;
            pos += bytesRead;
        }while((uint32_t) pos < size);
    }
    return pos;
}
int TCPServer::write(int cliSockfd, char *buff, size_t size, bool writeAll){
    int bytesRead = 0;
    int pos = 0;
    if(!writeAll){
        pos = ::write(cliSockfd, buff, size);
    }else{
        do{
            bytesRead = ::write(cliSockfd, &buff[pos], size - (uint32_t) pos);
            if(bytesRead < 0) return bytesRead;
            pos += bytesRead;
        }while((uint32_t) pos < size);
    }
    return pos;
}
int TCPServer::close(int sockfd){
    return ::close(sockfd);
}

int TCPServer::shutdown(int sockfd){
    int status;
    char buff[1];
//    status = ::shutdown(sockfd, how);
//    if(status < 0) return status;
    do{
        status = ::read(sockfd, buff, sizeof(buff));
        if(status < 0) return status;
    }while (status != 0);
    return ::close(sockfd);
}

int TCPServer::getFD(){
    return this->sockfd;
}

TCPClient::TCPClient(const char ip[], uint32_t port, bool blocking){
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
    return ::connect(this->sockfd, (sockaddr *) &this->addr, sizeof(this->addr));
}
int TCPClient::read(char *buff, size_t size, bool readAll){
    int bytesRead = 0;
    int pos = 0;
    if(!readAll){
        pos = ::read(this->sockfd, buff, size);
    }else{
        do{
            bytesRead = ::read(this->sockfd, &buff[pos], size - (uint32_t) pos);
            if(bytesRead < 0) return bytesRead;
            pos += bytesRead;
        }while((uint32_t) pos < size);
    }
    return pos;
}
int TCPClient::write(char *buff, size_t size, bool writeAll){
    int bytesRead = 0;
    int pos = 0;
    if(!writeAll){
        pos = ::write(this->sockfd, buff, size);
    }else{
        do{
            bytesRead = ::write(this->sockfd, &buff[pos], size - (uint32_t) pos);
            if(bytesRead < 0) return bytesRead;
            pos += bytesRead;
        }while((uint32_t) pos < size);
    }
    return pos;
}
int TCPClient::close(){
    return ::close(this->sockfd);
}
int TCPClient::getFD(){
    return this->sockfd;
}

