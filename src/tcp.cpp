#include "errors.h"
#include "ltdefs.h"
#include "tcp.h"

#include <arpa/inet.h>
#include <atomic>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <limits.h>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

ssize_t tcp::read(int sockfd,
                  char *buff, ssize_t size,
                  bool complete, bool blocking){
    int flags;
    ssize_t bytes, pos = 0;
    bool err;
     // Get fd flags
     flags = fcntl(sockfd, F_GETFL);
     if(flags < 0)return flags;
     // If blocking unset O_NONBLOCK
     if(blocking) flags = fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK);
     // Else set O_NONBLOCK
     else flags = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
     if(flags < 0) return flags;
     if(complete){
         // If complete read is required go into while loop
         while(pos < size){
             // Read bytes
             bytes = ::read(sockfd, &buff[pos], size - pos);
             err = bytes < 0;
             if(err) break;
             // Increment position to the next unread byte, or end if finished
             pos += bytes;
         }
         // Bytes = the number of bytes read
         if(!err) bytes = pos;
     }else{
         // Else perform basic best effort read
         bytes = ::read(sockfd, buff, size);
         err = bytes < 0;
     }
     if(err){
         // If it could not read...
         if(errno == EAGAIN || errno == EWOULDBLOCK) return 0;
         else return -1;
     }
     return bytes;
}

ssize_t tcp::write(int sockfd,
                   char *buff, ssize_t size,
                   bool complete, bool blocking){
    int flags;
    ssize_t bytes, pos = 0;
    bool err = false;
    // Get fd flags
    flags = fcntl(sockfd, F_GETFL);
    if(flags < 0)return flags;
    // If blocking unset O_NONBLOCK
    if(blocking) flags = fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK);
    // Else set O_NONBLOCK
    else flags = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    if(flags < 0) return flags;
    if(complete){
        // If complete read is required go into while loop
        while(pos < size){
            // Read bytes
            bytes = ::write(sockfd, &buff[pos], size - pos);
            err = bytes < 0;
            if(err) break;
            // Increment position to the next unread byte, or end if finished
            pos += bytes;
        }
        // Bytes = the number of bytes read
        if(!err) bytes = pos;
    }else{
        // Else perform basic best effort read
        bytes = ::write(sockfd, buff, size);
        err = bytes < 0;
    }
    if(err){
        // If it could not read...
        if(errno == EAGAIN || errno == EWOULDBLOCK) return 0;
        else return -1;
    }
    return bytes;
}

tcp::error::error(const std::string& what_arg): std::runtime_error(what_arg){

}

tcp::error::error(const char* what_arg): std::runtime_error(what_arg){

}

tcp::sys_error::sys_error(const std::string& what_arg): std::runtime_error(what_arg){

}

tcp::sys_error::sys_error(const char* what_arg): std::runtime_error(what_arg){

}

TCPServer::TCPServer(uint32_t port, bool blocking): clientCount(0){
    int status, flags;
    // Create socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(this->sockfd < 0) sysError("ERROR opening socket");
    // Set non-blocking flag if set to non-blocking
    flags = fcntl(this->sockfd, F_GETFL);
    if(flags < 0){
        error("Could not get flags");
        throw tcp::sys_error("Could not get flags");
    };
    if(blocking){
        // If blocking unset O_NONBLOCK flag
        flags = fcntl(this->sockfd, F_SETFL, flags & ~O_NONBLOCK);
    }else{
        // If non-blocking set O_NONBLOCK flag
        flags = fcntl(this->sockfd, F_SETFL, flags | O_NONBLOCK);
    }
    if(flags < 0){
        sysError("Could not set flags");
        throw tcp::sys_error("Could not set flags");
    };
    // Build address for server
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(port);
    this->addr.sin_addr.s_addr = INADDR_ANY;
    // Bind socket
    status = bind(this->sockfd,
                  (struct sockaddr *) &this->addr,
                  sizeof(this->addr));
    if(status < 0){
        sysError("ERROR binding socket");
        throw tcp::sys_error("Could not bind to socket");
    }
}

int TCPServer::getFD(sockaddr_in *client_addr){
    int fd;
    this->addrMapMtx.lock();
    // Check if address is in map
    AddrFDMap::iterator it = this->addrFDMap.find(addrIPv4ToString(client_addr));
    if(it == this->addrFDMap.end()){
        // If address is not in map error out
        error("Address not found");
        this->addrMapMtx.unlock();
        return ADDR_NOT_FOUND;
    }
    fd = it->second;
    this->addrMapMtx.unlock();
    return fd;
}

std::mutex* TCPServer::getMtx(sockaddr_in *client_addr){
    std::mutex *mtx;
    this->mtxMapMtx.lock();
    AddrMtxMap::iterator it = this->addrMtxMap.find(addrIPv4ToString(client_addr));
    if(it == this->addrMtxMap.end()){
        error("Address not found");
        this->mtxMapMtx.unlock();
        return NULL;
    }
    mtx = &it->second;
    this->mtxMapMtx.unlock();
    return mtx;
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
    this->addrMapMtx.lock();
    this->addrFDMap[addrIPv4ToString(client_addr)] = fd;
    this->addrMapMtx.unlock();
    this->mtxMapMtx.lock();
    this->addrMtxMap[addrIPv4ToString(client_addr)];
    this->mtxMapMtx.unlock();
    return 0;
}

ssize_t TCPServer::read(sockaddr_in *client_addr,
                        char *buff, ssize_t size, bool complete, bool blocking){
    int fd;
    ssize_t bytes;
    std::mutex *mtx;
    fd = this->getFD(client_addr);
    if(fd < 0) return fd;
    // Read socket
    mtx = this->getMtx(client_addr);
    if(mtx == NULL) return -1;
    mtx->lock();
    bytes = tcp::read(fd, buff, size, complete, blocking);
    mtx->unlock();
    return bytes;
}

ssize_t TCPServer::write(sockaddr_in *client_addr,
                         char *buff, ssize_t size, bool complete, bool blocking){
    int fd;
    ssize_t bytes;
    std::mutex *mtx;
    fd = this->getFD(client_addr);
    if(fd < 0) return fd;
    // Write socket
    mtx = this->getMtx(client_addr);
    if(mtx == NULL) return -1;
    mtx->lock();
    bytes = tcp::write(fd, buff, size, complete, blocking);
    mtx->unlock();
    return bytes;
}

uint32_t TCPServer::getClientCount(){
    return this->clientCount;
}

int TCPServer::closeCli(sockaddr_in *client_addr, bool force){
    int status, fd;
    AddrFDMap::iterator afdmIt;
    AddrMtxMap::iterator ammIt;
    std::mutex *mtx;
    fd = this->getFD(client_addr);
    if(fd < 0) return fd;
    // Decrement client counter
    this->clientCount--;
    // Get mutex for client
    mtx = this->getMtx(client_addr);
    if(mtx == NULL) return -1;
    // Close fd
    if(!force) mtx->lock();
    status = ::close(fd);
    if(status < 0) return status;
    if(!force) mtx->unlock();
    // Erase fd from map
    this->addrMapMtx.lock();
    afdmIt = this->addrFDMap.find(addrIPv4ToString(client_addr));
    this->addrFDMap.erase(afdmIt);
    this->addrMapMtx.unlock();
    this->mtxMapMtx.lock();
    ammIt = this->addrMtxMap.find(addrIPv4ToString(client_addr));
    this->addrMtxMap.erase(ammIt);
    this->mtxMapMtx.unlock();
    return status;
}

int TCPServer::close(bool force){
    int status;
    if(!force) this->mainSockMtx.lock();
    status = ::close(this->sockfd);
    if(!force) this->mainSockMtx.unlock();
    return status;
}

TCPClient::TCPClient(const char ip[], uint32_t port, bool blocking): connected(false){
    int status, flags;
    // Create socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(this->sockfd < 0) sysError("ERROR opening socket");
    // Get current flags
    flags = fcntl(this->sockfd, F_GETFL);
    if(flags < 0){
        sysError("Could not get flags");
        throw tcp::sys_error("Could not get flags");
    };
    if(blocking){
        // If blocking is required, unset O_NONBLOCK flag
        flags = fcntl(this->sockfd, F_SETFL, flags & ~O_NONBLOCK);
    }else{
        // Else set O_NONBLOCK flag
        flags = fcntl(this->sockfd, F_SETFL, flags | O_NONBLOCK);
    }
    if(flags < 0){
        sysError("Could not set flags");
        throw tcp::sys_error("Could not set flags");
    };
    // Create address to server
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(port);
    status = inet_aton(ip, &this->addr.sin_addr);
    if(status < 0){
        error("Error: Invalid IPv4 address");
        throw tcp::error("Invalid IPv4 address");
    }
}

int TCPClient::connect(){
    int status;
    // Connect to server
    this->rwMutex.lock();
    status = ::connect(this->sockfd, (sockaddr *) &this->addr, sizeof(this->addr));
    this->rwMutex.unlock();
    // Set connected flag to true if sucesfully connects
    this->connected = status == 0;
    return status;
}

ssize_t TCPClient::read(char *buff, ssize_t size, bool complete, bool blocking){
    ssize_t bytes;
    // Write socket
    this->rwMutex.lock();
    bytes = tcp::read(this->sockfd, buff, size, complete, blocking);
    this->rwMutex.unlock();
    return bytes;
}

ssize_t TCPClient::write(char *buff, ssize_t size, bool complete, bool blocking){
    ssize_t bytes;
    // Write socket
    this->rwMutex.lock();
    bytes = tcp::write(this->sockfd, buff, size, complete, blocking);
    this->rwMutex.unlock();
    return bytes;
}

int TCPClient::close(bool force){
    // Close socket connection
    if(!force) this->rwMutex.lock();
    return ::close(this->sockfd);
    if(!force) this->rwMutex.unlock();
}

int TCPClient::close(char *buff, ssize_t size, bool force){
    // Close socket connection
    if(!force) this->rwMutex.lock();
    int status = tcp::write(this->sockfd, buff, size);
    if(status < 0) return status;
    status = ::close(this->sockfd);
    if(!force) this->rwMutex.unlock();
    return status;
}

bool TCPClient::isConn(){
    // Returns state of socket (connected or not connected)
    return this->connected;
}
