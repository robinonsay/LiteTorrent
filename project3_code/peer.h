#ifndef PEER_H
#define PEER_H

#include "btldefs.h"

#include <fstream>
#include <netinet/in.h>
#include <list>
#include <map>
#include <thread>
#include <atomic>

#define PEER_TIMEOUT_ms 30000

class Peer{
    private:
        std::ofstream *outFile;
        std::ofstream *log;
        IP_ADDR addr;
        char *ip;
        std::list<IP_ADDR> peers;
        std::list<CHUNK_H> allChunkHdrs;
        std::map<uint32_t, CHUNK> *owndChunks;
        PACKET owndChunksPkt;
        std::list<CHUNK> chunks;
        std::list<std::thread> threads;
        int sockfd;
        std::atomic<bool> end;
        void server();
        void connHandler(int sockfd, IP_ADDR cliAddr);
        void chunkInqReqHandler(int sockfd, PACKET *pkt);
        void chunkReqHandler(int sockfd, PACKET *pkt);
        std::map<std::string, std::list<uint32_t>> chunkInq(int peerSockfd);
    public:
        Peer(char *myIP, char *trackerIP, std::map<uint32_t, CHUNK> *owndChunks,
             std::ofstream *outFile, std::ofstream *log);
        ~Peer();
        void run();
        void closePeer();
};

#endif

