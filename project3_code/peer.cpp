#include "peer.h"
#include "errors.h"
#include "btldefs.h"

#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <poll.h>

Peer::Peer(char *myIP, char *trackerIP, std::ifstream *owndChunksFile, std::ofstream *outFile, std::ofstream *log){
    int status, trckrSockfd;
    IP_ADDR trckr_addr;
    PACKET trrntFPkt;
    this->owndChunksFile = owndChunksFile;
    this->outFile = outFile;
    this->log = log;
    memset((char *) &trrntFPkt, 0, sizeof(trrntFPkt));
    trckrSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(trckrSockfd < 0) sysError("ERROR opening socket");
    memset((char *) &trckr_addr, 0, sizeof(trckr_addr));
    memset((char *) &this->addr, 0, sizeof(trckr_addr));
    trckr_addr.sin_family = AF_INET;
    trckr_addr.sin_port = htons(TRACKER_PORT);
    status = inet_aton(trackerIP, &trckr_addr.sin_addr);
    if(status == 0){
        fprintf(stderr, "ERROR invalid tracker IP address\n");
        exit(1);
    }
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(PEER_PORT);
    status = inet_aton(myIP, &this->addr.sin_addr);
    if(status == 0){
        fprintf(stderr, "ERROR invalid peer IP address\n");
        exit(1);
    }
    status = connect(trckrSockfd, (sockaddr *) &trckr_addr, sizeof(trckr_addr));
    if(status < 0) sysError("ERROR connecting to tracker");
    trrntFPkt.ph.type = TrrntFileReq;
    trrntFPkt.ph.length = 0;
    status = write(trckrSockfd, (char *) &trrntFPkt.ph, sizeof(trrntFPkt.ph));
    if(status < 0) sysError("ERROR writing to tracker socket file descriptor");
    memset((char *) &trrntFPkt, 0, sizeof(trrntFPkt));
    status = read(trckrSockfd, (char *) &trrntFPkt, sizeof(trrntFPkt));
    if(status < 0) sysError("ERROR reading from tracker socket file descriptor");
    if(trrntFPkt.ph.type == TrrntFileResp && trrntFPkt.ph.length > 0){
        printf("Recieved torrent file\n");
    }
//    std::string trrntFile (trrntFPkt.payload);
    std::istringstream iss (std::string(trrntFPkt.payload));
    int peerLen;
    iss >> peerLen;
    printf("%d\n", peerLen);
    IP_ADDR peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(PEER_PORT);
    for(int i=0; i < peerLen && iss.good(); i++){
        std::string currLine;
        iss >> currLine;
        printf("%s\n", currLine.c_str());
        status = inet_aton(currLine.c_str(), &peer_addr.sin_addr);
        if(status == 0){
            fprintf(stderr, "ERROR invalid peer IP address\n");
            exit(1);
        }
        this->peers.push_back(peer_addr);
    }
    int chunksLen;
    CHUNK chunk;
    iss >> chunksLen;
    printf("%d\n", chunksLen);
    for(int i=0; i < chunksLen*2 && iss.good(); i++){
        if(i % 2 == 0){
            iss >> chunk.index;
        }else {
            iss >> chunk.hash;
            this->allChunks.push_back(chunk);
            printf("%d %d\n", chunk.index, chunk.hash);
        }
    }
}

void Peer::run(){
}

