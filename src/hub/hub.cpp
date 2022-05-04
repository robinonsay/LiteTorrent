#include "hub/hub.h"
#include "tcp.h"
#include "errors.h"
#include "crc32.h"

#include <stdexcept>
#include <sstream>
#include <string>
#include <string.h>
#include <list>
#include <thread>
#include <arpa/inet.h>
#include <netinet/in.h>


Hub::Hub(std::istream& in_s, std::ostream& log_s): in(in_s), log(log_s){
    std::ostringstream tstream;
    std::string torrent;
    Chunk currChunk;
    std::list<ChunkHeader> chList;
    this->server = new TCPServer(HUB_PORT);
    for(int i=0; this->in.good(); i++){
        memset((char *) &currChunk, 0, sizeof(currChunk));
        this->in.read(currChunk.payload, sizeof(currChunk.payload));
        if(this->in.fail() && !this->in.eof()) throw std::runtime_error("Could not read input file");
        currChunk.ch.index = i;
        currChunk.ch.size = this->in.gcount();
        currChunk.ch.hash = crc32(currChunk.payload, sizeof(currChunk.payload));
        chList.push_back(currChunk.ch);
    }
    tstream << chList.size();
    for(std::list<ChunkHeader>::iterator it=chList.begin();
        it != chList.end(); ++it){
        tstream << it->index << ' ';
        tstream << it->size << ' ';
        tstream << it->hash << std::endl;
    }
    torrent = tstream.str();
    this->torrentPkt.ph.type = TFileResp;
    this->torrentPkt.ph.size = torrent.size();
    memcpy(this->torrentPkt.payload, torrent.c_str(), torrent.size());
}
Hub::~Hub(){
    delete this->server;
}
void Hub::close(){
    ThreadList::iterator th;
    for(th=this->threads.begin();th != this->threads.end(); ++th){
        th->join();
    }
    if(this->server->close(this->server->getFD()) < 0) sysError("ERROR closing socket");
    warning(this->log, "\nClosing Hub");
}
void Hub::run(){
    int status = this->server->listen(BACKLOG_SIZE);
    int peerSockfd;
    sockaddr_in peerAddr;
    size_t peerAddrLen = sizeof(peerAddr);
    if(status < 0) sysError("ERROR: TCP server listen failed");
    this->log << "Listening on port " << HUB_PORT << std::endl;
    while(1){
        peerSockfd = this->server->accept(&peerAddr, &peerAddrLen);
        if(peerSockfd < 0) sysError("ERROR: accepting connection");
        this->threads.push_back(std::thread(&Hub::connHandler, this, peerSockfd, peerAddr));
    }
}
void Hub::connHandler(int sockfd, sockaddr_in peerAddr){
    int status;
    this->log << "Accepted connection from " << inet_ntoa(peerAddr.sin_addr);
    this->log << ":"<< ntohs(peerAddr.sin_port) << std::endl;
    
    status = TCPServer::close(sockfd);
    if(status < 0) sysError("ERROR: closing peer socket");
    this->log << "Closing connection to " << inet_ntoa(peerAddr.sin_addr);
    this->log << ":"<< ntohs(peerAddr.sin_port) << std::endl;
}

