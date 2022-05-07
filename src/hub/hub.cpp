#include "lite_torrent/crypto.h"
#include "lite_torrent/errors.h"
#include "hub/hub.h"
#include "lite_torrent/mrsw_mutex.h"
#include "lite_torrent/tcp.h"

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
    int status;
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
        status = genHash("SHA3-256",
                         currChunk.payload,
                         sizeof(currChunk.payload),
                         &currChunk.ch.hash_s);
        if(status < 0){
            error("Could not generate hash", this->log);
            throw std::runtime_error("Could not generate hash");
        }
        chList.push_back(currChunk.ch);
    }
    // Create chunk header list used for payload
    torrent = new ChunkHeader[chList.size()];
    std::list<ChunkHeader>::iterator it;
    int i = 0;
    for(it=chList.begin(); it != chList.end(); ++it){
        // Chunk header at i gets chunk header in list
        torrent[i] = *it;
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
    this->pmMtx.lockRead();
    for(ami=this->peerAddrMap.begin(); ami != this->peerAddrMap.end(); ++ami){
        status = this->server.write(&ami->second, (char *) &finPkt, sizeof(finPkt));
        if(status < 0) sysError("ERROR: writing FIN to socket", this->log);
    }
    this->pmMtx.unlockRead();
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
            this->pmMtx.lockWrite();
            this->peerAddrMap[peerIPv4] = peerAddr;
            this->peerMap[peerIPv4];
            this->peersInMap = this->peerAddrMap.size();
            this->pmMtx.unlockWrite();
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
        if(pkt.ph.type == FIN && pkt.ph.size == 0){
            // If FIN erase peer from map and indicate that connection is done
            info("Recieved FIN", this->log);
            this->pmMtx.lockWrite();
            this->peerAddrMap.erase(peerIPv4);
            this->peerMap.erase(peerIPv4);
            this->peersInMap = this->peerAddrMap.size();
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
    PeerMap::iterator pmIt;
    std::list<ChunkHeader>::iterator chlIt;
    char *buffer;
    size_t bufferSize;
    std::string pktStr;
    peerCount = this->server.getClientCount();
    // Only loop while the Hub is running
    while(!this->closing){
        if(peerCount != this->server.getClientCount() &&
           this->peersInMap == this->server.getClientCount()){
            memset(&pkt, 0, sizeof(pkt));
            pktStream.str("");
            pktStream.clear();
            this->pmMtx.lockRead();
            info("Peers:", this->log);
            for(pmIt=this->peerMap.begin(); pmIt != this->peerMap.end(); ++pmIt){
                this->log << '\t' << pmIt->first << std::endl;
                pktStream << pmIt->first << std::endl;
                pktStream << pmIt->second.size() << std::endl;
                bufferSize = pmIt->second.size() * sizeof(ChunkHeader);
                buffer = new char[bufferSize];
                int i=0;
                for(chlIt=pmIt->second.begin(); chlIt != pmIt->second.end(); ++chlIt){
                    memcpy(&buffer[i], &*chlIt, sizeof(ChunkHeader));
                    i += sizeof(ChunkHeader);
                }
                pktStream << buffer << std::endl;
                delete buffer;
            }
            pktStr = pktStream.str();
            pkt.ph.type = UPDATE;
            pkt.ph.size = pktStr.size();
            memcpy(pkt.payload, pktStr.c_str(), pktStr.size());
            info("Sending update to:", this->log);
            for(amIt=this->peerAddrMap.begin();
                amIt != this->peerAddrMap.end(); ++amIt){
                this->log << '\t' << amIt->first << std::endl;
                status = this->server.write(&amIt->second,(char *) &pkt, sizeof(pkt));
                if(status < 0) sysError("Could not write packet to peer", this->log);
            }
            this->pmMtx.unlockRead();
            peerCount = this->server.getClientCount();
        }
    }
}
