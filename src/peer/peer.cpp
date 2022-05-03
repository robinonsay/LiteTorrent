#include "peer/peer.h"
#include "errors.h"
#include "btldefs.h"
#include "crc32.h"

#include <iostream>
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
#include <stdlib.h>
#include <mutex>


bool comp(CHUNK& lhs, CHUNK& rhs){
    return lhs.ch.index < rhs.ch.index;
}

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
    if(status == 0) error(&std::cerr, "ERROR invalid tracker IP address");
    status = connect(trckrSockfd, (sockaddr *) &trckr_addr, sizeof(trckr_addr));
    if(status < 0) sysError("ERROR connecting to tracker");
    trrntFPkt.ph.type = TrrntFileReq;
    trrntFPkt.ph.length = 0;
    status = write(trckrSockfd, (char *) &trrntFPkt.ph, sizeof(trrntFPkt.ph));
    if(status < 0) sysError("ERROR writing to tracker socket file descriptor");
    (*this->log) << inet_ntoa(trckr_addr.sin_addr) << " " << TrrntFileReq << " " << 0 << std::endl;
    memset((char *) &trrntFPkt, 0, sizeof(trrntFPkt));
    status = read(trckrSockfd, (char *) &trrntFPkt, sizeof(trrntFPkt));
    if(status < 0) sysError("ERROR reading from tracker socket file descriptor");
    if(trrntFPkt.ph.type == TrrntFileResp && trrntFPkt.ph.length > 0){
        printf("Recieved torrent file\n");
        (*this->log) << inet_ntoa(trckr_addr.sin_addr) << " " << TrrntFileResp << " " << trrntFPkt.ph.length << std::endl;
    }
    if(close(trckrSockfd) < 0) sysError("ERROR closing tracker socket");
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
        if(status == 0) error(&std::cerr, "ERROR invalid peer IP address");
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
            this->chunkHashMap[chunk.hash] = chunk.index;
            this->chunkIDMap[chunk.index] = chunk.hash;
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
    if(status == 0) error(&std::cerr, "ERROR invalid peer IP address");
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

std::map<uint32_t, std::list<std::string>> Peer::chunkInq(){
    PACKET pkt;
    int status, peerSockfd;
    IP_ADDR peer;
    std::map<uint32_t, std::list<std::string>> chunkMap;
    for(std::list<IP_ADDR>::iterator it=this->peers.begin();
        it != this->peers.end(); ++it){
        memset((char *) &peer, 0, sizeof(peer));
        pkt.ph.type = ChunkInqReq;
        pkt.ph.length = 0;
        peerSockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(peerSockfd < 0) sysError("ERROR opening socket");
        peer = *it;
        std::string peerIP (inet_ntoa(peer.sin_addr));
        if(strcmp(inet_ntoa(peer.sin_addr), this->ip) == 0) continue;
        bool tryAgain;
        printf("Requesting chunk list from %s\n", peerIP.c_str());
        do{
            status = connect(peerSockfd, (sockaddr *) &peer, sizeof(peer));
            tryAgain = status < 0 && errno == ECONNREFUSED;
        } while(tryAgain);
        if(status < 0) sysError("ERROR connecting to peer");
        status = this->write_p(peerSockfd, (char *) &pkt, sizeof(pkt));
        if(status < 0) sysError("ERROR writing chunk inquiry packet");
        (*this->log) << inet_ntoa(peer.sin_addr) << " " << ChunkInqReq << " " << 0 << std::endl;
        status = this->read_p(peerSockfd, (char *) &pkt, sizeof(pkt));
        if(status < 0) sysError("ERROR reading chunk inquiry response");
        if(pkt.ph.type == ChunkInqResp && pkt.ph.length > 0){
            printf("Recieved chunk list from %s\n", peerIP.c_str());
            (*this->log) << inet_ntoa(peer.sin_addr) << " " << ChunkInqResp << " " << pkt.ph.length << std::endl;
            std::list<uint32_t> chunkIDs;
            uint32_t chunkID;
            for(unsigned int i=0; i<pkt.ph.length; i+=sizeof(chunkID)){
                memcpy(&chunkID, &pkt.payload[i], sizeof(chunkID));
                chunkIDs.push_back(chunkID);
            }
            chunkIDs.sort();
            for(std::list<uint32_t>::iterator it=chunkIDs.begin();
                it != chunkIDs.end(); ++it){
                try{
                    chunkMap.at(*it).push_back(peerIP);
                }catch(const std::out_of_range& oor){
                    std::list<std::string> ipList;
                    ipList.push_back(peerIP);
                    chunkMap[*it] = ipList;
                }
            }
        }
        if(close(peerSockfd) < 0) sysError("ERROR couldn't close peer socket");
    }
    return chunkMap;
}

int Peer::reqChunk(std::string peerIP, uint32_t hash, CHUNK *chunk){
    int status, peerSockfd, bytesRead;
    IP_ADDR peer_addr;
    PACKET pkt;
    memset((char *) &pkt, 0, sizeof(pkt));
    pkt.ph.type = ChunkReq;
    pkt.ph.length = sizeof(hash);
    memcpy(pkt.payload, &hash, sizeof(hash));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(PEER_PORT);
    printf("Requesting Chunk %u from %s\n", hash, peerIP.c_str());
    status = inet_aton(peerIP.c_str(), &peer_addr.sin_addr);
    if(status == 0) error(&std::cerr, "ERROR invalid peer IP address");
    peerSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(peerSockfd < 0) sysError("ERROR opening socket");
    status = connect(peerSockfd, (sockaddr *) &peer_addr, sizeof(peer_addr));
    if(status < 0){
        printf("Could not connect to %s to get chunk %u\n", inet_ntoa(peer_addr.sin_addr), hash);
        if(close(peerSockfd) < 0) sysError("ERROR couldn't close peer socket");
        return -1;
    }
    status = this->write_p(peerSockfd, (char *) &pkt, sizeof(pkt));
    if(status < 0){
        printf("Could not write chunk request pkt to %s\n", inet_ntoa(peer_addr.sin_addr));
        if(close(peerSockfd) < 0) sysError("ERROR couldn't close peer socket");
        return -1;
    }
    (*this->log) << inet_ntoa(peer_addr.sin_addr) << " " << ChunkReq << " " << pkt.ph.length << " " << hash << std::endl;
    memset((char *) &pkt, 0, sizeof(pkt));
    bytesRead = this->read_p(peerSockfd, (char *) &pkt, sizeof(pkt));
    if(bytesRead < 0) sysError("ERROR couldn't read peer socket");
    uint32_t calcHash = crc32(pkt.payload, CHUNK_SIZE);
    if(pkt.ph.type == ChunkResp && pkt.ph.length > 0 && hash == calcHash){
        (*this->log) << inet_ntoa(peer_addr.sin_addr) << " " << ChunkResp << " " << pkt.ph.length << " " << hash<< std::endl;
        memcpy(chunk->payload, pkt.payload, pkt.ph.length);
        chunk->ch.index = this->chunkHashMap[hash];
        chunk->ch.hash = hash;
        chunk->ch.length = pkt.ph.length;
        if(close(peerSockfd) < 0) sysError("ERROR couldn't close peer socket");
        return bytesRead;
    }else{
        printf("Invalid chunk response from %s\n", inet_ntoa(peer_addr.sin_addr));
        if(close(peerSockfd) < 0) sysError("ERROR couldn't close peer socket");
        return -1;
    }

}

void Peer::run(){
    int status;
    CHUNK chunk;
    std::map<uint32_t, std::list<std::string>> chunkMap;
    std::mutex mtx;
    this->threads.push_back(std::thread(&Peer::server, this));
    chunkMap = this->chunkInq();
    mtx.lock();
    for(std::map<uint32_t, CHUNK>::iterator it=this->owndChunks->begin();
        it != this->owndChunks->end(); ++it){
        this->chunks.push_back(it->second);
        chunkMap.erase(it->second.ch.index);
    }
    mtx.unlock();
    printf("Chunk Map:\n");
    for(std::map<uint32_t, std::list<std::string>>::iterator it=chunkMap.begin();
        it != chunkMap.end(); ++it){
        printf("\t%u -> %s\n", it->first, it->second.front().c_str());
    }
    while(!chunkMap.empty()){
        std::map<uint32_t, std::list<std::string>>::iterator it = chunkMap.begin();
        uint32_t rarestChunk_id = it->first;
        ++it;
        while(it != chunkMap.end()){
            if(chunkMap[rarestChunk_id].size() > it->second.size()){
                rarestChunk_id = it->first;
            }
            ++it;
        }
        status = this->reqChunk(chunkMap[rarestChunk_id].front(), this->chunkIDMap[rarestChunk_id], &chunk);
        if(status < 0)  error(&std::cerr, "Chunk Request Failed");
        this->chunks.push_back(chunk);
        chunkMap.erase(rarestChunk_id);
    }
    this->chunks.sort(comp);
    for(std::list<CHUNK>::iterator it=this->chunks.begin();
        it != this->chunks.end(); ++it){
        this->outFile->write(it->payload, it->ch.length);
    }
    while(1){
        sleep(1);
    }
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

void Peer::connHandler(int cliSockfd, IP_ADDR cliAddr){
    int status, numEvents;
    const int SIZE = 1;
    bool timeout = false;
    bool recvPkt = false;
    struct pollfd pfds[SIZE];
    PACKET pkt;
    pfds[0].fd = cliSockfd;
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
            status = this->read_p(cliSockfd, (char *) &pkt, sizeof(pkt));
            if(status < 0) sysError("ERROR reading cli socket");
            if(pkt.ph.type == ChunkInqReq && pkt.ph.length == 0){
                (*this->log) << inet_ntoa(cliAddr.sin_addr) << " " << ChunkInqReq << " " << 0 << std::endl;
                this->chunkInqReqHandler(cliSockfd);
                (*this->log) << inet_ntoa(cliAddr.sin_addr) << " " << ChunkInqResp << " " << this->owndChunksPkt.ph.length << std::endl;
                printf("Wrote chunk list to %s\n", inet_ntoa(cliAddr.sin_addr));
                recvPkt = true;
            }else if(pkt.ph.type == ChunkReq && pkt.ph.length == sizeof(uint32_t)){
                uint32_t reqHash;
                memcpy(&reqHash, pkt.payload, sizeof(uint32_t));
                (*this->log) << inet_ntoa(cliAddr.sin_addr) << " " << ChunkReq << " " << pkt.ph.length << " " << reqHash<< std::endl;
                this->chunkReqHandler(cliSockfd, &pkt);
                (*this->log) << inet_ntoa(cliAddr.sin_addr) << " " << ChunkResp << " " << pkt.ph.length << " " << reqHash << std::endl;
                printf("Wrote chunk to %s\n", inet_ntoa(cliAddr.sin_addr));
                recvPkt = true;
            }
        }
    }
    status = close(cliSockfd);
    if(status < 0) sysError("ERROR closing cli socket");
    printf("Connection to peer at %s closed\n", inet_ntoa(cliAddr.sin_addr));
}

void Peer::chunkInqReqHandler(int cliSockfd){
    int status;
    status = this->write_p(cliSockfd, (char *) &this->owndChunksPkt, sizeof(this->owndChunksPkt));
    if(status < 0) sysError("ERROR writing owned chunks packet");
}

void Peer::chunkReqHandler(int cliSockfd, PACKET *pkt){
    uint32_t reqHash;
    int status;
    std::mutex mtx;
    memcpy(&reqHash, pkt->payload, sizeof(uint32_t));
    memset((char *) pkt, 0, sizeof(PACKET));
    pkt->ph.type = ChunkResp;
    mtx.lock();
    CHUNK reqChunk = this->owndChunks->at(reqHash);
    mtx.unlock();
    pkt->ph.length = reqChunk.ch.length;
    memcpy(pkt->payload, reqChunk.payload, sizeof(pkt->payload));
    status = this->write_p(cliSockfd, (char *) pkt, sizeof(PACKET));
    if(status < 0) sysError("ERROR writing chunk packet");
    printf("Bytes written: %d\n", status);
}

int Peer::write_p(int fd, char *buffer, size_t size){
    int bytesRead = 0;
    uint32_t pos = 0;
    do{
        bytesRead = write(fd, &buffer[pos], size-pos);
        if(bytesRead < 0) return bytesRead;
        pos += (uint32_t) bytesRead;
    }while(pos < size);
    return (int) pos;
}
int Peer::read_p(int fd, char *buffer, size_t size){
    int bytesRead = 0;
    uint32_t pos = 0;
    do{
        bytesRead = read(fd, &buffer[pos], size-pos);
        if(bytesRead < 0) return bytesRead;
        pos += (uint32_t) bytesRead;
    }while(pos < size);
    return (int) pos;
}

