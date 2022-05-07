#include "lite_torrent/errors.h"
#include "lite_torrent/ltdefs.h"
#include "lite_torrent/mrsw_mutex.h"
#include "peer/errors.h"
#include "peer/peer.h"
#include "lite_torrent/ltpeer.h"
#include "lite_torrent/tcp.h"

#include <atomic>
#include <errno.h>
#include <iostream>
#include <list>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <unistd.h>



Peer::Peer(const char myIP[], const char hubIP[],
           std::ostream& out_s, std::ostream& log_s):
           closing(false), log(log_s), out(out_s),
           hub(hubIP, HUB_PORT), peerServer(PEER_PORT){
    this->myIP = myIP;
}

Peer::~Peer(){
}

void Peer::run(){
    this->open();
    this->close();
}

void Peer::open(){
    int status;
    bool isFIN = false;
    Packet pkt;
    this->threads.push_back(std::thread(&Peer::server, this));
    // Connect to hub
    status = this->hub.connect();
    // Error out if a connection couldn't be established to hub
    if(status < 0 && errno != EISCONN){
        sysError("Error connecting to hub");
        throw p::sys_error("Error connecting to hub");
    }
    info("Connected to hub", this->log);
    // Request torrent
    memset((char *) &pkt, 0, sizeof(pkt));
    status = this->reqTorrent();
    if(status < 0) error("Could not request torrent", this->log);
    do{
        // Set packet to 0
        memset((char *) &pkt, 0, sizeof(pkt));
        status = this->hub.read((char *) &pkt.ph, sizeof(pkt.ph), true, false);
        if(status < 0){
            sysError("Could not read packet from hub", this->log);
            break;
        }
        if(status == 0) continue;
        if(pkt.ph.type == FIN && pkt.ph.size == 0){
            isFIN = true;
        }else if(pkt.ph.type == TRRNT_RESP && pkt.ph.size > 0){
            isFIN = false;
            status = this->hub.read(pkt.payload, sizeof(pkt.payload), true);
            if(status < 0){
                sysError("Could not read packet", this->log);
                break;
            }
            this->parseTorrent(&pkt);
        }else if(pkt.ph.type == UPDATE && pkt.ph.size > 0){
            isFIN = false;
            status = this->hub.read(pkt.payload, sizeof(pkt.payload), true);
            if(status < 0){
                sysError("Could not read packet", this->log);
                break;
            }
            this->update(&pkt);
        }
    }while(!isFIN);
}

int Peer::reqTorrent(){
    int status;
    PacketHeader trrntReq;
    // Check if connected
    if(!this->hub.isConn()){
        error("Hub not connected", this->log);
        return -1;
    }
    // Set packet to 0
    // Form torrent request
    trrntReq.type = TRRNT_REQ;
    trrntReq.size = 0;
    // Write torrent request to hub
    status = this->hub.write((char *) &trrntReq, sizeof(trrntReq));
    if(status < 0){
        sysError("Could not write torrent request", this->log);
        return -1;
    }
    return status;
}

void Peer::parseTorrent(Packet *pkt){
    int numCHs;
    ChunkHeader chunkHeader;
    // Determine number of chunk headers
    numCHs = pkt->ph.size / sizeof(ChunkHeader);
    int n = 0;  // Chunk header location in payload
    for(int i=0; i < numCHs; i++){
        // Calculate chunk header location in payload
        n = i * sizeof(ChunkHeader);
        // Set temp chunk header to 0
        memset(&chunkHeader, 0 , sizeof(chunkHeader));
        // Copy memory to temp chunk header
        memcpy(&chunkHeader, &pkt->payload[n], sizeof(ChunkHeader));
        // TODO: Remove debugging statement
        this->log << chunkHeader.index << ' ' << chunkHeader.size << std::endl;
        // Push chunk header onto torrent linked list
        this->torrent.push_back(chunkHeader);
    }
}

void Peer::update(Packet *pkt){
    std::istringstream pktStream(pkt->payload);
    std::string ipv4Str;
    size_t len;
    ChunkHeader currCH;
    info("Recieved Update", this->log);
    while(pktStream >> ipv4Str){
        this->log << "IPv4 string:" << std::endl;
        this->log << '\t' << ipv4Str << std::endl;
        pktStream >> len;
        this->log << "Length:" << std::endl;
        this->log << '\t' << len << std::endl;
        for(size_t i=0; i < len; i++){
            pktStream.read((char *) &currCH, sizeof(currCH));
            this->pchmMtx.lockWrite();
            this->peerMap[ipv4Str] = LtPeer(ipv4Str);
            this->peerMap[ipv4Str].addChunk(currCH);
            this->pchmMtx.unlockWrite();
        }
    }
}

void Peer::close(bool interrupt){
    int status;
    PacketHeader finHdr = {FIN, 0};
    this->log << std::endl;
    // Write FIN packet to hub
    info("FIN sending", this->log);
    status = this->hub.close((char *) &finHdr, sizeof(finHdr), interrupt);
    if(status < 0){
        sysError("Couldn't close hub connection", this->log);
        throw p::sys_error("Couldn't close hub connection");
    }
    info("FIN sent", this->log);
    // Notify other threads of closing
    this->closing = true;
    // Join threads
    ThreadList::iterator thi;
    for(thi=this->threads.begin(); thi != this->threads.end(); ++thi){
        thi->join();
    }
}

void Peer::server(){
    while(!this->closing){
        sleep(1);
    }
    warning("Closing peer server", this->log);
}

void Peer::connHandler(sockaddr_in peerAddr){
}
