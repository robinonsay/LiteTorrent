#include "tcp.h"
#include "errors.h"
#include "ltdefs.h"

#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <stdexcept>

TCPServer::TCPServer(uint32_t port, bool blocking){
    int status;
    // Create socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(this->sockfd < 0) sysError("ERROR opening socket");
    // Set non-blocking flag if set to non-blocking
    if(!blocking) fcntl(this->sockfd, F_SETFL, O_NONBLOCK);
    // Build address for server
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(port);
    this->addr.sin_addr.s_addr = INADDR_ANY;
    // Bind socket
    status = bind(this->sockfd,
                  (struct sockaddr *) &this->addr,
                  sizeof(this->addr));
    if(status < 0) sysError("ERROR binding socket");
}
int TCPServer::listen(uint32_t backlog){
    // Listen for incoming connections
    return ::listen(this->sockfd, backlog);
}
int TCPServer::accept(sockaddr_in *client_addr, size_t *addrlen){
    // Accept incoming connection
    int fd;
    fd = ::accept(this->sockfd,
                  (struct sockaddr *) client_addr,
                  (socklen_t *) addrlen);
    if(fd < 0) return fd;
    this->addrFDMap[addrIPv4ToString(client_addr)] = fd;
    return fd;
}
int TCPServer::read(sockaddr_in *client_addr, char *buff, size_t size, bool readAll){
    AddrFDMap::iterator it = this->addrFDMap.find(addrIPv4ToString(client_addr));
    if(it == this->addrFDMap.end()){
        error("Address not found");
        return -1;
    }
    return ::read(it->second, buff, size);
}
int TCPServer::write(sockaddr_in *client_addr, char *buff, size_t size, bool writeAll){
    AddrFDMap::iterator it = this->addrFDMap.find(addrIPv4ToString(client_addr));
    if(it == this->addrFDMap.end()){
        error("Address not found");
        return -1;
    }
    return ::write(it->second, buff, size);
}

uint32_t TCPServer::getClientCount(){
    return this->clientCount;
}

int TCPServer::close(sockaddr_in *client_addr){
    AddrFDMap::iterator it = this->addrFDMap.find(addrIPv4ToString(client_addr));
    if(it == this->addrFDMap.end()){
        error("Address not found");
        return -1;
    }
    return ::close(it->second);
}

int TCPServer::shutdown(sockaddr_in *client_addr){
    // Orderly shutdown of socket
    int status;
    char buff[1];
    // Empty read buffer
    do{
        status = this->read(client_addr, buff, sizeof(buff));
        if(status < 0) return status;
    }while (status != 0);
    // Close socket
    return this->close(client_addr);
}

int TCPServer::getFD(){
    // Gets sock fd
    return this->sockfd;
}

TCPClient::TCPClient(const char ip[], uint32_t port, bool blocking): connected(false){
    int status;
    // Create socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(this->sockfd < 0) sysError("ERROR opening socket");
    // Sets socket to non-blocking if non-blocking is indicated
    if(!blocking) fcntl(this->sockfd, F_SETFL, O_NONBLOCK);
    // Create address to server
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(port);
    status = inet_aton(ip, &this->addr.sin_addr);
    if(status < 0){
        error("Error: Invalid IPv4 address");
        throw std::runtime_error("Invalid IPv4 address");
    }
}
int TCPClient::connect(){
    int status;
    // Connect to server
    status = ::connect(this->sockfd, (sockaddr *) &this->addr, sizeof(this->addr));
    // Set connected flag to true if sucesfully connects
    this->connected = status >= 0;
    return status;
}
int TCPClient::read(char *buff, size_t size, bool readAll){
    // Read socket
    return ::read(this->sockfd, buff, size);
}
int TCPClient::write(char *buff, size_t size, bool writeAll){
    // Write socket
    return ::write(this->sockfd, buff, size);
}
int TCPClient::close(){
    // Close socket connection
    return ::close(this->sockfd);
}
int TCPClient::getFD(){
    // Get socket file desciptor
    return this->sockfd;
}

bool TCPClient::isConn(){
    // Returns state of socket (connected or not connected)
    return this->connected;
}
