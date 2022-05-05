#include "tcp.h"
#include "errors.h"
#include "ltdefs.h"

#include <atomic>
#include <mutex>
#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <stdexcept>
#include <limits.h>
#include <errno.h>

TCPServer::TCPServer(uint32_t port, bool blocking){
    int status, flags;
    // Create socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(this->sockfd < 0) sysError("ERROR opening socket");
    // Set non-blocking flag if set to non-blocking
    flags = fcntl(this->sockfd, F_GETFL);
    if(flags < 0){
        error("Could not get flags");
        perror("ERROR");
    };
    if(blocking){
        // If blocking unset O_NONBLOCK flag
        flags = fcntl(this->sockfd, F_SETFL, flags & ~O_NONBLOCK);
    }else{
        // If non-blocking set O_NONBLOCK flag
        flags = fcntl(this->sockfd, F_SETFL, flags | O_NONBLOCK);
    }
    if(flags < 0){
        error("Could not set flags");
        perror("ERROR");
    };
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
    this->clientCount++;
    this->mapMutex.lock();
    this->addrFDMap[addrIPv4ToString(client_addr)] = fd;
    this->mapMutex.unlock();
    return 0;
}

ssize_t TCPServer::read(sockaddr_in *client_addr,
                        char *buff, ssize_t size, bool complete, bool blocking){
    int fd, flags;
    ssize_t bytes;
    ssize_t pos = 0;
    this->mapMutex.lock();
    // Check if address is in map
    AddrFDMap::iterator it = this->addrFDMap.find(addrIPv4ToString(client_addr));
    if(it == this->addrFDMap.end()){
        // If address is not in map error out
        error("Address not found");
        this->mapMutex.unlock();
        return -1;
    }
    fd = it->second;
    this->mapMutex.unlock();
    // Write socket
    this->rwMutex.lock();
    // Get fd flags
    flags = fcntl(fd, F_GETFL);
    if(flags < 0){
        this->rwMutex.unlock();
        return flags;
    }
    if(blocking){
        // If blocking unset O_NONBLOCK flag
        flags = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }else{
        // Else set O_NONBLOCK flag
        flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    if(flags < 0) {
        this->rwMutex.unlock();
        return flags;
    }
    if(complete){
        // If complete read is required go into while loop
        while(pos < size){
            // Read bytes
            bytes = ::read(fd, &buff[pos], size - pos);
            if(bytes < 0){
                // If it could not read...
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    // If it was going to block, release mutex and return 0
                    this->rwMutex.unlock();
                    return 0;
                }else{
                    // Else return -1 (errno is set)
                    this->rwMutex.unlock();
                    return bytes;
                }
            }
            // Increment position to the next unread byte, or end if finished
            pos += bytes;
        }
        // Bytes = the number of bytes read
        bytes = pos;
    }else{
        // Else perform basic best effort read
        bytes = ::read(fd, buff, size);
    }
    this->rwMutex.unlock();
    return bytes;
}

ssize_t TCPServer::write(sockaddr_in *client_addr,
                         char *buff, ssize_t size, bool complete, bool blocking){
    int fd, flags;
    ssize_t bytes;
    ssize_t pos = 0;
    this->mapMutex.lock();
    // Check if address is in map
    AddrFDMap::iterator it = this->addrFDMap.find(addrIPv4ToString(client_addr));
    if(it == this->addrFDMap.end()){
        // If address is not in map return -1
        error("Address not found");
        this->mapMutex.unlock();
        return -1;
    }
    fd = it->second;
    this->mapMutex.unlock();
    // Read socket
    this->rwMutex.lock();
    // Get current flags set
    flags = fcntl(fd, F_GETFL);
    if(flags < 0){
        this->rwMutex.unlock();
        return flags;
    }
    if(blocking){
        // If blocking unset O_NONBLOCK flag
        flags = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }else{
        // Else set O_NONBLOCK flag
        flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    if(flags < 0){
        this->rwMutex.unlock();
        return flags;
    }
    if(complete){
        // If complete write is required go into while loop
        while(pos < size){
            // Write bytes
            bytes = ::write(fd, &buff[pos], size - pos);
            if(bytes < 0){
                // If bytes were not written
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    // And if it would have blocked, return 0
                    this->rwMutex.unlock();
                    return 0;
                }else{
                    // Otherwise set errno and return -1
                    this->rwMutex.unlock();
                    return bytes;
                }
            }
            pos += bytes;
        }
        // Bytes = number of bytes written
        bytes = pos;
    }else{
        // Else write bytes with simple best effort write
        bytes = ::write(fd, buff, size);
    }
    this->rwMutex.unlock();
    return bytes;
}

uint32_t TCPServer::getClientCount(){
    return this->clientCount;
}

int TCPServer::closeCli(sockaddr_in *client_addr){
    int status, fd;
    this->mapMutex.lock();
    // Check if address is in map
    AddrFDMap::iterator it = this->addrFDMap.find(addrIPv4ToString(client_addr));
    if(it == this->addrFDMap.end()){
        error("Address not found");
        this->mapMutex.unlock();
        return -1;
    }
    // Get fd
    fd = it->second;
    this->mapMutex.unlock();
    // Decrement client counter
    this->clientCount--;
    // Close fd
    status = ::close(fd);
    this->mapMutex.lock();
    // Erase fd from map
    if (status >= 0) this->addrFDMap.erase(it);
    this->mapMutex.unlock();
    return status;
}

int TCPServer::shutdownCli(sockaddr_in *client_addr){
    // Orderly shutdown of socket
    int status;
    char buff[1];
    // Empty read buffer
    do{
        status = this->read(client_addr, buff, sizeof(buff), false);
        if(status < 0) return status;
    }while (status != 0);
    // Close socket
    return this->closeCli(client_addr);
}

int TCPServer::close(){
    return ::close(this->sockfd);
}

int TCPServer::getFD(){
    // Gets sock fd
    return this->sockfd;
}

TCPClient::TCPClient(const char ip[], uint32_t port, bool blocking): connected(false){
    int status, flags;
    // Create socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(this->sockfd < 0) sysError("ERROR opening socket");
    // Get current flags
    flags = fcntl(this->sockfd, F_GETFL);
    if(flags < 0){
        error("Could not get flags");
        perror("ERROR");
    };
    if(blocking){
        // If blocking is required, unset O_NONBLOCK flag
        flags = fcntl(this->sockfd, F_SETFL, flags & ~O_NONBLOCK);
    }else{
        // Else set O_NONBLOCK flag
        flags = fcntl(this->sockfd, F_SETFL, flags | O_NONBLOCK);
    }
    if(flags < 0){
        error("Could not set flags");
        perror("ERROR");
    };
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
    this->connected = status == 0;
    return status;
}
ssize_t TCPClient::read(char *buff, ssize_t size, bool complete){
    ssize_t bytes;
    ssize_t pos = 0;
    // Write socket
    this->rwMutex.lock();
    if(complete){
        // If complete is required go into while loop
        while(pos < size){
            // Read bytes
            bytes = ::read(this->sockfd, &buff[pos], size - pos);
            if(bytes < 0){
                this->rwMutex.unlock();
                return bytes;
            }
            pos += bytes;
        }
        // bytes = the total number of bytes read
        bytes = pos;
    }else{
        // Else perform simple read best effort
        bytes = ::read(this->sockfd, buff, size);
    }
    this->rwMutex.unlock();
    return bytes;
}
ssize_t TCPClient::write(char *buff, ssize_t size, bool complete){
    ssize_t bytes;
    ssize_t pos = 0;
    // Write socket
    this->rwMutex.lock();
    if(complete){
        // If complete write is required go into while loop
        while(pos < size){
            // Write bytes
            bytes = ::write(this->sockfd, &buff[pos], size - pos);
            if(bytes < 0){
                this->rwMutex.unlock();
                return bytes;
            }
            pos += bytes;
        }
        // bytes = the total number of bytes written
        bytes = pos;
    }else{
        // Else perform simple write best effort
        bytes = ::write(this->sockfd, buff, size);
    }
    this->rwMutex.unlock();
    return bytes;
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
