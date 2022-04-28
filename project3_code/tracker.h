#ifndef TRACKER_H
#define TRACKER_H

#include <fstream>
#include <list>
#include <netinet/in.h>

#define PORT 6969

typedef struct sockaddr_in IP_ADDR;

class Tracker{
    private:
        std::list<IP_ADDR> ipAddrs;
    public:
        Tracker(std::ifstream *pListFile, std::ifstream *inFile,
                std::ofstream *tFile, std::ofstream *log);
};
#endif

