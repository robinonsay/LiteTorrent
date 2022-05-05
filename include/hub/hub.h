#ifndef HUB_H
#define HUB_H

#include "ltdefs.h"
#include "tcp.h"

#include <map>
#include <list>
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>

typedef std::list<std::thread> ThreadList;
typedef std::map<std::string, std::list<ChunkHeader>> AddrChunkMap;
typedef std::map<std::string, int> AddrFDMap;


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
        TCPServer server;
        ThreadList threads;
        AddrChunkMap peerMap;
        AddrFDMap peerFDMap;
        std::mutex peerMapMutex;
        std::mutex peerFDMapMutex;
        std::atomic<bool> closing;
        std::atomic<uint32_t> numPeers;
        void connHandler(int sockfd, sockaddr_in peerAddr);
};

#endif

