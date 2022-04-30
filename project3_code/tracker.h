#ifndef TRACKER_H
#define TRACKER_H

#include "btldefs.h"

#include <fstream>
#include <list>
#include <netinet/in.h>
#include <thread>

#define TIMEOUT_ms 60000

typedef struct sockaddr_in IP_ADDR;

typedef struct chunk{
    unsigned int index;
    uint32_t hash;
} CHUNK;

class Tracker{
    private:
        std::list<IP_ADDR> ipAddrs;
        std::list<CHUNK> chunks;
        std::list<std::thread> threads;
        std::ofstream *log;
        PACKET trrntPkt;
        IP_ADDR tracker_addr;
        int sockfd;
        void connHandler(int sockfd, IP_ADDR cliAddr);
    public:
        Tracker(char *pListPath, char *tFilePath, char *inFilePath, std::ofstream *log);
        ~Tracker();
        void run();
};
#endif

