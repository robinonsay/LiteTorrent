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
        std::map<unsigned int, CHUNK> *owndChunks;
        std::list<CHUNK> chunks;
    public:
        Peer(char *myIP, char *trackerIP, std::map<unsigned int, CHUNK> *owndChunks,
             std::ofstream *outFile, std::ofstream *log);
        void run();
};

#endif

