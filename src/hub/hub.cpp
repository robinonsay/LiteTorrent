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
    int status;
    ThreadList::iterator th;
    PeerfdList::iterator pfd;
    PacketHeader finPkt = {FIN, 0};
    for(th=this->threads.begin(); th != this->threads.end(); ++th){
        th->join();
    }
    this->log << std::endl;
    this->log << "Closing client connections..." << std::endl;
    for(pfd=this->peerFDs.begin(); pfd != this->peerFDs.end(); ++pfd){
        status = TCPServer::write(*pfd, (char *) &finPkt, sizeof(finPkt), false);
        if(status < 0) sysError("ERROR: writing FIN to socket");
//        do{
//            memset((char *) &finPkt, 0, sizeof(finPkt));
//            status = TCPServer::read(*pfd, &finPkt, sizeof(finPkt), false);
//            if(status < 0) sysError("ERROR: reading FIN to socket");
//        } while(finPkt.type != FIN);
        status = TCPServer::shutdown(*pfd);
        if(status < 0) sysError("ERROR: shutting down peer socket");
    }
    if(TCPServer::close(this->server->getFD()) < 0) sysError("ERROR closing socket");
    warning(this->log, "Hub Closed");
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
        if(peerSockfd < 0)
            sysError("ERROR: accepting connection");
        else
            this->threads.push_back(std::thread(&Hub::connHandler, this, peerSockfd, peerAddr));
    }
}
void Hub::connHandler(int sockfd, sockaddr_in peerAddr){
    int status;
    std::ostringstream addrStream;
    std::list<ChunkHeader> chList;
    addrStream << inet_ntoa(peerAddr.sin_addr) << ":" << ntohs(peerAddr.sin_port);
    this->log << "Accepted connection from " << addrStream.str() << std::endl;
    this->mutex.lock();
    this->clientMap[addrStream.str()] = chList;
    this->peerFDs.push_back(sockfd);
    this->mutex.unlock();

}

