#ifndef PEER_H
#define PEER_H

#include "btldefs.h"

#include <fstream>
#include <netinet/in.h>
#include <list>
#include <map>

class Peer{
    private:
        std::ofstream *outFile;
        std::ofstream *log;
        IP_ADDR addr;
        std::list<IP_ADDR> peers;
        std::list<CHUNK_H> allChunks;
        std::map<uint32_t, CHUNK> *owndChunks;
        std::list<CHUNK> chunks;
        int sockfd;
    public:
        Peer(char *myIP, char *trackerIP, std::map<uint32_t, CHUNK> *owndChunks,
             std::ofstream *outFile, std::ofstream *log);
        void run();
};

#endif

