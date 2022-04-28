#ifndef TRACKER_H
#define TRACKER_H

#include "btldefs.h"

#include <fstream>
#include <list>
#include <netinet/in.h>

typedef struct sockaddr_in IP_ADDR;

typedef struct chunk{
    unsigned int index;
    uint32_t hash;
} CHUNK;

class Tracker{
    private:
        std::list<IP_ADDR> ipAddrs;
        std::list<CHUNK> chunks;
        std::ofstream *log;
        IP_ADDR tracker_addr;
        int sockfd;
    public:
        Tracker(char *pListPath, char *tFilePath, char *inFilePath, std::ofstream *log);
        ~Tracker();
};
#endif

