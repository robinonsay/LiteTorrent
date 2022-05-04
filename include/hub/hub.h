#ifndef HUB_H
#define HUB_H

#include "ltdefs.h"
#include "tcp.h"

#include <list>
#include <thread>
#include <mutex>
#include <iostream>

typedef std::list<std::thread> ThreadList;
typedef std::map<std::string, ChunkHeader> AddrChunkMap;

#define TIMEOUT 30000  // in ms
#define BACKLOG_SIZE 10

class Hub{
    public:
        Hub(std::istream& in_s=std::cin, std::ostream& log_s=std::cout);
        ~Hub();
        void close();
        void run();
    private:
        std::istream& in;
        std::ostream& log;
        Packet torrentPkt;
        TCPServer *server;
        ThreadList threads;
        AddrChunkMap clientMap;
        std::mutex mutex;
        void connHandler(int sockfd, sockaddr_in peerAddr);
};

#endif

