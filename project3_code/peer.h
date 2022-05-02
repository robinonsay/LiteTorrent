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

bool comp(CHUNK& lhs, CHUNK& rhs);

class Peer{
    private:
        std::ofstream *outFile;
        std::ofstream *log;
        IP_ADDR addr;
        char *ip;
        std::list<IP_ADDR> peers;
        std::map<uint32_t, uint32_t> chunkHashMap;
        std::map<uint32_t, uint32_t> chunkIDMap;
        std::map<uint32_t, CHUNK> *owndChunks;
        PACKET owndChunksPkt;
        std::list<CHUNK> chunks;
        std::list<std::thread> threads;
        int sockfd;
        std::atomic<bool> end;
        void server();
        void connHandler(int cliSockfd, IP_ADDR cliAddr);
        void chunkInqReqHandler(int cliSockfd);
        void chunkReqHandler(int cliSockfd, PACKET *pkt);
        std::map<uint32_t, std::list<std::string>> chunkInq();
        int reqChunk(std::string peerIP, uint32_t hash, CHUNK *chunk);
        int write_p(int fd, char *buffer, size_t size);
        int read_p(int fd, char *buffer, size_t size);
    public:
        Peer(char *myIP, char *trackerIP, std::map<uint32_t, CHUNK> *owndChunks,
             std::ofstream *outFile, std::ofstream *log);
        ~Peer();
        void run();
        void closePeer();
};

#endif

