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
#include <stdio.h>


Hub::Hub(std::istream& in_s, std::ostream& log_s): in(in_s), log(log_s), server(HUB_PORT), closing(false), numPeers(0){
    Chunk currChunk;
    std::list<ChunkHeader> chList;
    ChunkHeader *torrent;
    // Chunk input stream
    for(int i=0; this->in.good(); i++){
        memset((char *) &currChunk, 0, sizeof(currChunk));
        this->in.read(currChunk.payload, sizeof(currChunk.payload));
        if(this->in.fail() && !this->in.eof()) throw std::runtime_error("Could not read input file");
        currChunk.ch.index = i;
        currChunk.ch.size = this->in.gcount();
        currChunk.ch.hash = crc32(currChunk.payload, sizeof(currChunk.payload));
        chList.push_back(currChunk.ch);
    }
    // Create chunk header list used for payload
    info(this->log, "Torrent:");
    torrent = new ChunkHeader[chList.size()];
    std::list<ChunkHeader>::iterator it;
    int i = 0;
    for(it=chList.begin(); it != chList.end(); ++it){
        // Chunk header at i gets chunk header in list
        torrent[i] = *it;
        this->log << it->index << ' ';
        this->log << it->size << ' ';
        this->log << it->hash << std::endl;
        i++;
    }
    // Form torrent response and create packet
    this->torrentPkt.ph.type = TRRNT_RESP;
    this->torrentPkt.ph.size = chList.size() * sizeof(ChunkHeader);
    memcpy(this->torrentPkt.payload, torrent, this->torrentPkt.ph.size);
}

Hub::~Hub(){
}

void Hub::close(){
    int status;
    ThreadList::iterator th;
    AddrFDMap::iterator pfd;
    // Create FIN packet
    PacketHeader finPkt = {FIN, 0};
    // Notify other threads of closing
    this->closing = true;
    this->log << std::endl;
    warning(this->log, "Closing client connections...");
    // Write FIN to all peers
    // Connection handler will take care of closing in a clean manner
    for(pfd=this->peerFDMap.begin(); pfd != this->peerFDMap.end(); ++pfd){
        status = TCPServer::write(pfd->second, (char *) &finPkt, sizeof(finPkt));
        if(status < 0) sysError("ERROR: writing FIN to socket");
    }
    // Join threads
    for(th=this->threads.begin(); th != this->threads.end(); ++th){
        th->join();
    }
    // Close main fd connection
    if(TCPServer::close(this->server.getFD()) < 0) sysError("ERROR closing socket");
    warning(this->log, "Hub Closed");
}
void Hub::run(){
    int status;
    int peerSockfd;
    sockaddr_in peerAddr;
    size_t peerAddrLen = sizeof(peerAddr);
    std::ostringstream addrStream;
    // Listen for incoming connections
    status = this->server.listen(BACKLOG_SIZE);
    if(status < 0) sysError("ERROR: TCP server listen failed");
    this->log << "Listening on port " << HUB_PORT << std::endl;
    while(1){
        // Accept the connection
        peerSockfd = this->server.accept(&peerAddr, &peerAddrLen);
        if(peerSockfd < 0)
            sysError("ERROR: accepting connection");
        else{
            // Create address string stream
            addrStream.str(std::string());
            // Push connection to its own thread via the connection handler
            this->threads.push_back(std::thread(&Hub::connHandler, this, peerSockfd, peerAddr));
            addrStream << inet_ntoa(peerAddr.sin_addr) << ":" << ntohs(peerAddr.sin_port);
            this->log << "Accepted connection from " << addrStream.str() << std::endl;
            // Add peer to peer maps
            this->peerMap[addrStream.str()] = std::list<ChunkHeader>();
            this->peerFDMap[addrStream.str()] = peerSockfd;
        }
    }
}

void Hub::connHandler(int sockfd, sockaddr_in peerAddr){
    int status;
    bool isFIN = false;
    std::ostringstream addrStream;
    Packet pkt;
    // Create address string for lookup
    addrStream << inet_ntoa(peerAddr.sin_addr) << ":" << ntohs(peerAddr.sin_port);
    // Increment atomic variable number of peers
    this->numPeers++;
    do{
        // Clear packet
        memset((char *) &pkt, 0, sizeof(pkt));
        // Read data into packet
        status = TCPServer::read(sockfd, (char *) &pkt, sizeof(pkt));
        if(status < 0){
            error(this->log, "ERROR reading from client sockfd");
            perror("ERROR");
            break;
        }
        if(pkt.ph.type == FIN && pkt.ph.size == 0){
            // If FIN erase peer from map and indicate that connection is done
            this->peerFDMap.erase(addrStream.str());
            this->peerMap.erase(addrStream.str());
            isFIN = true;
        }else if(pkt.ph.type == TRRNT_REQ && pkt.ph.size == 0){
            // If torrent request respond with torrent
            this->log << "Torrent request from: " << addrStream.str() << std::endl;
            status = TCPServer::write(sockfd, (char *) &this->torrentPkt, sizeof(this->torrentPkt));
            if(status < 0){
                error(this->log, "ERROR writing torrent to client sockfd");
                perror("ERROR");
                break;
            }
        }
    }while(!isFIN);
    // Connection is finished, shutdown (close) connection to peer
    if(TCPServer::shutdown(sockfd) < 0){
        error(this->log, "ERROR closing client sockfd");
        perror("ERROR");
    }
    // Decrement number of peers
    this->numPeers--;
}

