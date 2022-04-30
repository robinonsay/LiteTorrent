#include "peer.h"
#include "errors.h"
#include "btldefs.h"

#include <fstream>
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
    PACKET trrntFilePkt;
    this->owndChunksFile = owndChunksFile;
    this->outFile = outFile;
    this->log = log;
    memset((char *) &trrntFilePkt, 0, sizeof(trrntFilePkt));
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
    trrntFilePkt.ph.type = TrrntFileReq;
    trrntFilePkt.ph.length = 0;
    status = write(trckrSockfd, (char *) &trrntFilePkt.ph, sizeof(trrntFilePkt.ph));
    if(status < 0) sysError("ERROR writing to tracker socket file descriptor");
    memset((char *) &trrntFilePkt, 0, sizeof(trrntFilePkt));
    status = read(trckrSockfd, (char *) &trrntFilePkt, sizeof(trrntFilePkt));
    if(status < 0) sysError("ERROR reading from tracker socket file descriptor");
    if(trrntFilePkt.ph.type == TrrntFileResp && trrntFilePkt.ph.length > 0){
        printf("Recieved torrent file\n");
    }
}

void Peer::run(){
}

