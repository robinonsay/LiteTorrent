#include "tracker.h"
#include "crc32.h"

#include <string>
#include <fstream>
#include <list>
#include <netinet/in.h>
#include <arpa/inet.h>

Tracker::Tracker(std::ifstream *pListFile, std::ifstream *inFile,
                 char *tFilePath, std::ofstream *log){
    int status;
    std::string ip_str;
    std::ofstream tFile (tFilePath, std::ofstream::out);
    IP_ADDR peer_addr;
    CHUNK chunk;
    char chunkBuff[CHUNK_SIZE];
    if(!tFile.is_open()){
        printf("Torrent file could not be opened. Check path\n");
        exit(1);
    }
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
    tFile << this->ipAddrs.size() << std::endl;
    for(std::list<IP_ADDR>::iterator it=this->ipAddrs.begin(); it != this->ipAddrs.end(); ++it){
        tFile << inet_ntoa(it->sin_addr) << std::endl;
    }
    for(int i=0; !inFile->eof(); i++){
        inFile->read(chunkBuff, CHUNK_SIZE);
        if(inFile->fail() && !inFile->eof()){
            fprintf(stderr, "ERROR could not read input file\n");
            exit(1);
        }
        chunk.index = i;
        chunk.hash = crc32(chunkBuff, CHUNK_SIZE);
        this->chunks.push_back(chunk);
    }
    tFile << this->chunks.size() << std::endl;
    for(std::list<CHUNK>::iterator it=this->chunks.begin(); it != this->chunks.end(); ++it){
        tFile << it->index << ' ' << it->hash << std::endl;
    }
    tFile.close();
}

