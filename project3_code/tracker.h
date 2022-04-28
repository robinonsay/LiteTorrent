#ifndef TRACKER_H
#define TRACKER_H

#include <fstream>
#include <list>
#include <netinet/in.h>

#define PORT 6969
#define CHUNK_SIZE 512000

typedef struct sockaddr_in IP_ADDR;

typedef struct chunk{
    unsigned int index;
    uint32_t hash;
} CHUNK;

class Tracker{
    private:
        std::list<IP_ADDR> ipAddrs;
        std::list<CHUNK> chunks;
    public:
        Tracker(std::ifstream *pListFile, std::ifstream *inFile,
                std::ofstream *tFile, std::ofstream *log);
};
#endif

