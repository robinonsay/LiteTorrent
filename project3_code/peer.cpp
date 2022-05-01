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
#include <atomic>
#include <fcntl.h>
#include <errno.h>

Peer::Peer(char *myIP, char *trackerIP, std::map<uint32_t, CHUNK> *owndChunks,
           std::ofstream *outFile, std::ofstream *log){
    int status, trckrSockfd;
    IP_ADDR trckr_addr;
    PACKET trrntFPkt;
    this->outFile = outFile;
    this->log = log;
    this->owndChunks = owndChunks;
    this->end = false;
    this->ip = myIP;
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
    std::istringstream iss (std::string(trrntFPkt.payload));
    int peerLen;
    iss >> peerLen;
    printf("Num Peers: %d\n", peerLen);
    IP_ADDR peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(PEER_PORT);
    for(int i=0; i < peerLen && iss.good(); i++){
        std::string currLine;
        iss >> currLine;
        status = inet_aton(currLine.c_str(), &peer_addr.sin_addr);
        if(status == 0){
            fprintf(stderr, "ERROR invalid peer IP address\n");
            exit(1);
        }
        this->peers.push_back(peer_addr);
    }
    int chunksLen;
    CHUNK_H chunk;
    iss >> chunksLen;
    printf("Num Chunks: %d\n", chunksLen);
    for(int i=0; i < chunksLen*2 && iss.good(); i++){
        if(i % 2 == 0){
            iss >> chunk.index;
        }else {
            iss >> chunk.hash;
            this->allChunkhs.push_back(chunk);
        }
    }
    this->owndChunksPkt.ph.type = ChunkInqResp;
    this->owndChunksPkt.ph.length = this->owndChunks->size() * sizeof(uint32_t);
    int i = 0;
    for(std::map<uint32_t, CHUNK>::iterator it=this->owndChunks->begin();
        it != this->owndChunks->end(); ++it){
        memcpy(&this->owndChunksPkt.payload[i], &it->second.ch.index, sizeof(it->second.ch.index));
        i += sizeof(it->second.ch.index);
    }
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(PEER_PORT);
    status = inet_aton(myIP, &this->addr.sin_addr);
    if(status == 0){
        fprintf(stderr, "ERROR invalid peer IP address\n");
        exit(1);
    }
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(this->sockfd, F_SETFL, O_NONBLOCK);
    if(this->sockfd < 0) sysError("ERROR opening socket");
    status = bind(this->sockfd,
                  (struct sockaddr *) &this->addr,
                  sizeof(this->addr));
    if(status < 0) sysError("ERROR on binding");
}

Peer::~Peer(){
}

void Peer::run(){
    PACKET pkt;
    int status, peerSockfd = socket(AF_INET, SOCK_STREAM, 0);
    IP_ADDR peer;
    this->threads.push_back(std::thread(&Peer::server, this));
    pkt.ph.type = ChunkInqReq;
    pkt.ph.length = 0;
    for(std::list<IP_ADDR>::iterator it=this->peers.begin();
        it != this->peers.end(); ++it){
        peer = *it;
        if(strcmp(inet_ntoa(peer.sin_addr), this->ip) == 0) continue;
        bool tryAgain;
        do{
            status = connect(peerSockfd, (sockaddr *) &peer, sizeof(peer));
            tryAgain = status < 0 && errno == ECONNREFUSED;
            if(tryAgain) sleep(5);
        } while(tryAgain);
        if(status < 0) sysError("ERROR connecting to peer");
        status = write(peerSockfd, (char *) &pkt.ph, sizeof(pkt.ph));
        if(status < 0) sysError("ERROR writing chunk inquiry packet");
        status = read(peerSockfd, (char *) &pkt, sizeof(pkt));
        if(status < 0) sysError("ERROR reading chunk inquiry response");
        printf("Type: %d\nLength: %d\n", pkt.ph.type, pkt.ph.length);
    }
    while(1);
}

void Peer::closePeer(){
    this->end = true;
    for(std::list<std::thread>::iterator th=this->threads.begin();
        th != this->threads.end(); ++th){
        th->join();
    }
    if(close(this->sockfd) < 0) sysError("ERROR closing socket");
    printf("\nPeer Closed\n");
}

void Peer::server(){
    int connSockfd, cliAddrLen, status;
    IP_ADDR cliAddr;
    cliAddrLen = sizeof(cliAddr);
    status = listen(this->sockfd, BACKLOG_QUEUE_SIZE);
    if(status < 0) sysError("ERROR on listen");
    printf("Listening on port %d\n", ntohs(this->addr.sin_port));
    while(!this->end){
        connSockfd = accept(this->sockfd, (struct sockaddr *) &cliAddr, (socklen_t *) &cliAddrLen);
        if(connSockfd > 0){
            this->threads.push_back(std::thread(&Peer::connHandler, this, connSockfd, cliAddr));
        }else if(errno != EAGAIN && errno != EWOULDBLOCK){
            sysError("ERROR accepting connection");
        }
    }
}

void Peer::connHandler(int sockfd, IP_ADDR cliAddr){
    int status, numEvents;
    const int SIZE = 1;
    bool timeout = false;
    bool recvPkt = false;
    struct pollfd pfds[SIZE];
    PACKET pkt;
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    memset((char *) &pkt, 0, sizeof(pkt));
    printf("Accepting connection from peer at %s\n", inet_ntoa(cliAddr.sin_addr));
    while(!(timeout || recvPkt)){
        numEvents = poll(pfds, SIZE, PEER_TIMEOUT_ms);
        if(numEvents < 0){
            sysError("ERROR polling cli socket");
        } else if(numEvents == 0){
            // timeout
            timeout = true;
        }else if((pfds[0].revents & POLLIN) == 1){
            // Data on fd
            status = read(sockfd, (char *) &pkt, sizeof(pkt));
            if(status < 0) sysError("ERROR reading cli socket");
            if(pkt.ph.type == ChunkInqReq && pkt.ph.length == 0){
                this->chunkInqReqHandler(sockfd, &pkt);
                recvPkt = true;
            }else if(pkt.ph.type == ChunkReq && pkt.ph.length != 0){
                this->chunkReqHandler(sockfd, &pkt);
                recvPkt = true;
            }
        }
    }
    status = close(sockfd);
    if(status < 0) sysError("ERROR closing cli socket");
    printf("Connection to peer at %s closed\n", inet_ntoa(cliAddr.sin_addr));
}

void Peer::chunkInqReqHandler(int sockfd, PACKET *pkt){
    int status;
    status = write(sockfd, (char *) &this->owndChunksPkt, sizeof(this->owndChunksPkt));
    if(status < 0) sysError("ERROR writing owned chunks packet");
}

void Peer::chunkReqHandler(int sockfd, PACKET *pkt){

}

