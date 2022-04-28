#include "tracker.h"

#include <string>
#include <fstream>
#include <list>
#include <netinet/in.h>
#include <arpa/inet.h>

Tracker::Tracker(std::ifstream *pListFile, std::ifstream *inFile,
                 std::ofstream *tFile, std::ofstream *log){
    int status;
    std::string ip_str;
    IP_ADDR peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(PORT);
    while(!pListFile->eof()){
        *pListFile >> ip_str;
        status = inet_aton(ip_str.c_str(), &peer_addr.sin_addr);
        if(status == 0){
            fprintf(stderr, "ERROR invalid IP address\n");
            exit(1);
        }
        this->ipAddrs.push_back(peer_addr);
    }
    *tFile << this->ipAddrs.size() << std::endl;
    for(std::list<IP_ADDR>::iterator it=this->ipAddrs.begin(); it != this->ipAddrs.end(); ++it){
        *tFile << inet_ntoa(it->sin_addr) << std::endl;
    }
}

