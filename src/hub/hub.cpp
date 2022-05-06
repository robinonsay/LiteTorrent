#include "crc32.h"
#include "errors.h"
#include "hub/hub.h"
#include "mutex/mrsw_mutex.h"
#include "tcp.h"

#include <arpa/inet.h>
#include <list>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>


Hub::Hub(std::istream& in_s, std::ostream& log_s): in(in_s), log(log_s), server(HUB_PORT), closing(false){
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
    info("Torrent:", this->log);
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

void Hub::close(bool interrupt){
    int status;
    ThreadList::iterator th;
    AddrMap::iterator ami;
    // Create FIN packet
    PacketHeader finPkt = {FIN, 0};
    // Notify other threads of closing
    this->closing = true;
    this->log << std::endl;
    warning("Closing client connections...", this->log);
    // Write FIN to all peers
    // Connection handler will take care of closing in a clean manner
    this->pamMtx.lockRead();
    for(ami=this->peerAddrMap.begin(); ami != this->peerAddrMap.end(); ++ami){
        status = this->server.write(&ami->second, (char *) &finPkt, sizeof(finPkt));
        if(status < 0) sysError("ERROR: writing FIN to socket", this->log);
    }
    this->pamMtx.unlockRead();
    // Join threads
    for(th=this->threads.begin(); th != this->threads.end(); ++th){
        th->join();
    }
    // Close main fd connection
    if(this->server.close() < 0) sysError("ERROR closing socket", this->log);
    warning("Hub Closed", this->log);
}
void Hub::run(){
    int status;
    sockaddr_in peerAddr;
    size_t peerAddrLen = sizeof(peerAddr);
    std::string peerIPv4;
    // Listen for incoming connections
    status = this->server.listen(BACKLOG_SIZE);
    if(status < 0) sysError("ERROR: TCP server listen failed", this->log);
    this->log << "Listening on port " << HUB_PORT << std::endl;
    this->threads.push_back(std::thread(&Hub::updatePeers, this));
    while(1){
        // Accept the connection
        status = this->server.accept(&peerAddr, &peerAddrLen);
        if(status < 0)
            sysError("ERROR: accepting connection", this->log);
        else{
            // Create address string stream
            peerIPv4 = addrIPv4ToString(&peerAddr);
            this->log << "Accepted connection from " << peerIPv4 << std::endl;
            // Push connection to its own thread via the connection handler
            this->threads.push_back(std::thread(&Hub::peerConnHandler, this, peerIPv4));
            // Add peer to peer maps
            this->pamMtx.lockWrite();
            this->pmMtx.lockWrite();
            this->peerAddrMap[peerIPv4] = peerAddr;
            this->peerCHMap[peerIPv4] = std::list<ChunkHeader>();
            this->peersInMap = this->peerAddrMap.size();
            this->pmMtx.unlockWrite();
            this->pamMtx.unlockWrite();
        }
    }
}

void Hub::peerConnHandler(std::string peerIPv4){
    int status;
    bool isFIN = false;
    Packet pkt;
    sockaddr_in *peerAddr;
    peerAddr = &this->peerAddrMap[peerIPv4];
    do{
        // Clear packet
        memset((char *) &pkt, 0, sizeof(pkt));
        // Read data into packet
        status = this->server.read(peerAddr,
                                   (char *) &pkt.ph, sizeof(pkt.ph),
                                   true, false);
        if(status < 0){
            sysError("ERROR reading from client sockfd", this->log);
            break;
        }
        if(status == 0) continue;
        this->log << "Bytes read: " << status << std::endl;
        if(pkt.ph.type == FIN && pkt.ph.size == 0){
            // If FIN erase peer from map and indicate that connection is done
            info("Recieved FIN", this->log);
            this->pamMtx.lockWrite();
            this->pmMtx.lockWrite();
            this->peerAddrMap.erase(peerIPv4);
            this->peerCHMap.erase(peerIPv4);
            this->peersInMap = this->peerAddrMap.size();
            this->pamMtx.unlockWrite();
            this->pmMtx.unlockWrite();
            isFIN = true;
        }else if(pkt.ph.type == TRRNT_REQ && pkt.ph.size == 0){
            // If torrent request respond with torrent
            this->log << "Torrent request from: " << peerIPv4 << std::endl;
            status = this->server.write(peerAddr,
                                        (char *) &this->torrentPkt,
                                        sizeof(this->torrentPkt));
            if(status < 0){
                sysError("ERROR writing torrent to client sockfd", this->log);
                break;
            }
        }
    }while(!isFIN);
    // Connection is finished, shutdown (close) connection to peer
    status = this->server.closeCli(peerAddr);
    if(status < 0){
        sysError("ERROR closing client sockfd", this->log);
    }
}

void Hub::updatePeers(){
    int status;
    uint32_t peerCount;
    Packet pkt;
    std::ostringstream pktStream;
    AddrMap::iterator amIt;
    AddrChunkMap::iterator acmIt;
    std::list<ChunkHeader>::iterator chlIt;
    char *buffer;
    size_t bufferSize;
    std::string pktStr;
    peerCount = this->server.getClientCount();
    // Only loop while the Hub is running
    pkt.ph.type = UPDATE;
    while(!this->closing){
        if(peerCount != this->server.getClientCount() &&
           this->peersInMap == this->server.getClientCount()){
            this->log << "Peer count: " << this->server.getClientCount() << std::endl;
            pktStream.str("");
            pktStream.clear();
            this->pmMtx.lockRead();
            for(acmIt=this->peerCHMap.begin(); acmIt != this->peerCHMap.end(); ++acmIt){
                pktStream << acmIt->first << std::endl;
                pktStream << acmIt->second.size() << std::endl;
                bufferSize = acmIt->second.size() * sizeof(ChunkHeader);
                buffer = new char[bufferSize];
                int i=0;
                for(chlIt=acmIt->second.begin(); chlIt != acmIt->second.end(); ++chlIt){
                    memcpy(&buffer[i], &*chlIt, sizeof(ChunkHeader));
                    i += sizeof(ChunkHeader);
                }
                pktStream << buffer << std::endl;
                delete buffer;
            }
            this->pmMtx.unlockRead();
            pktStr = pktStream.str();
            pkt.ph.size = pktStr.size();
            memcpy(pkt.payload, pktStr.c_str(), pktStr.size());
            this->pamMtx.lockRead();
            for(amIt=this->peerAddrMap.begin();
                amIt != this->peerAddrMap.end(); ++amIt){
                this->log << "Sending update to:\t" << amIt->first << std::endl;
                status = this->server.write(&amIt->second,(char *) &pkt, sizeof(pkt));
                if(status < 0) sysError("Could not write packet to peer", this->log);
            }
            this->pamMtx.unlockRead();
            info("UPDATEs sent", this->log);
            peerCount = this->server.getClientCount();
        }
    }
}
