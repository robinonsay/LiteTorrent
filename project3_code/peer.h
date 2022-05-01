#ifndef PEER_H
#define PEER_H

#include "btldefs.h"

#include <fstream>
#include <netinet/in.h>
#include <list>

class Peer{
    private:
        std::ifstream *owndChunksFile;
        std::ofstream *outFile;
        std::ofstream *log;
        IP_ADDR addr;
        std::list<IP_ADDR> peers;
        std::list<CHUNK> allChunks;
    public:
        Peer(char *myIP, char *trackerIP, std::ifstream *owndChunksFile, std::ofstream *outFile, std::ofstream *log);
        void run();
};

#endif

