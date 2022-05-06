#ifndef TCP_H
#define TCP_H

#include "ltdefs.h"
#include "mutex/mrsw_mutex.h"

#include <atomic>
#include <map>
#include <stdexcept>
#include <sys/socket.h>

#define ADDR_NOT_FOUND -500

/** Address-to-FD mapping type*/
typedef std::map<std::string, int> AddrFDMap;

/** Address-to-mutex map */
typedef std::map<std::string, std::mutex> AddrMtxMap;

namespace tcp{
    /**
    * Reads from socket file descriptor
    * @param sockfd The socket file desciptor
    * @param buff The buffer to read in to
    * @param size The size of the read
    * @param readAll Flag to read the whole size
    * @return status of the read() system call;
    * returns bytes read on success, -1 on failure and sets errno
    */
    ssize_t read(int sockfd,
                 char *buff, ssize_t size,
                 bool complete=true, bool blocking=true);

    /**
    * Write from socket file descriptor
    * @param sockfd The socket file desciptor
    * @param buff The buffer to write from
    * @param size The size of the write
    * @param readAll Flag to write the whole size
    * @return status of the write() system call;
    * returns bytes written on success, -1 on failure and sets errno
    */
    ssize_t write(int sockfd,
              char *buff, ssize_t size,
              bool complete=true, bool blocking=true);


}

/** TCPServer Class */
class TCPServer{
    private:
        int sockfd;  /** Socket file descriptor */
        sockaddr_in addr;  /** Server address */
        AddrFDMap addrFDMap;
        AddrMtxMap addrMtxMap;
        std::atomic<uint32_t> clientCount;  /** Counts the number of clients */
        std::mutex mainSockMtx;  /** Synchronizes reads and writes */
        MRSWMutex addrMapMtx;  /** Synchronizes map access */
        MRSWMutex mtxMapMtx;

        /** Gets FD for given client address */
        int getFD(sockaddr_in *client_addr);

        /** Gets Mutex for given client address */
        std::mutex* getMtx(sockaddr_in *client_addr);
    public:

        /**
        * TCPServer constructor
        * @param port Port of Server
        * @param blocking Flag for if the server is blocking; defaults to true
        */
        TCPServer(uint32_t port, bool blocking=true);

        /**
        * Listens to incoming connections
        * @param backlog Queue size for incoming connections
        * @return status of system call to listen();
        * returns 0 on success, -1 on failure and sets errno
        */
        int listen(uint32_t backlog);

        /**
        * Accepts incoming connections
        * @param client_addr Client address pointer. Write the client address to this
        * pointer's value
        * @param addrlen Length of the client address after reading the address
        * @return status of accept() system call;
        * returns 0 on success, -1 on failure and sets errno
        */
        int accept(sockaddr_in *client_addr, size_t *addrlen);

        /**
        * Reads from client socket file descriptor
        * @param client_addr The client address to read
        * @param buff The buffer to read in to
        * @param size The size of the read
        * @param readAll Flag to read the whole size
        * @return status of the read() system call;
        * returns bytes read on success, -1 on failure and sets errno
        */
        ssize_t read(sockaddr_in *client_addr,
                     char *buff, ssize_t size,
                     bool complete=true, bool blocking=true);

        /**
        * Writes to client socket file descriptor
        * @param client_addr The client address to write
        * @param buff The buffer to write from
        * @param size The size of the write
        * @param readAll Flag to write the whole size
        * @return status of the write() system call;
        * returns bytes written on success, -1 on failure and sets errno
        */
        ssize_t write(sockaddr_in *client_addr,
                  char *buff, ssize_t size,
                  bool complete=true, bool blocking=true);

        /** Returns number of clients connected to the server atomically */
        size_t getClientCount();

        /**
        * Closes the Socket
        * @param client_addr The client address to Close
        * @return status of the close() system call;
        * returns 0 on success, -1 on failure and sets errno
        */
        int closeCli(sockaddr_in *client_addr, bool force=false);

        /** Closes server */
        int close(bool force=false);
};

class TCPClient{
    private:
        int sockfd;  /** Socket file descriptor */
        sockaddr_in addr;  /** Server socket address */
        std::atomic<bool> connected;  /** Connection state */
        std::mutex rwMutex;  /** Mutex synchronizing reads and writes */
    public:

        /**
        * TCPClient constructor
        * @param ip IPv4 address string of server
        * @param port Port of server
        * @param Blocking flag for socket
        */
        TCPClient(const char ip[], uint32_t port, bool blocking=true);

        /**
        * Connects to server
        */
        int connect();

        /**
        * Read from server socket file descriptor
        * @param buff The buffer to read in to
        * @param size The size of the read
        * @param readAll Flag to read the whole size
        * @return status of the read() system call;
        * returns bytes read on success, -1 on failure and sets errno
        */
        ssize_t read(char *buff, ssize_t size,
                     bool complete=true, bool blocking=true);

        /**
        * Write from server socket file descriptor
        * @param buff The buffer to write in to
        * @param size The size of the write
        * @param readAll Flag to write the whole size
        * @return status of the write() system call;
        * returns bytes read on success, -1 on failure and sets errno
        */
        ssize_t write(char *buff, ssize_t size,
                      bool complete=true, bool blocking=true);

        /** Close socket connection to server */
        int close(bool force=false);

        /** Close by sending a packet first */
        int close(char *buff, ssize_t size, bool force=false);

        /** Check connection state atomically*/
        bool isConn();
};

#endif
