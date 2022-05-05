#include "peer/peer.h"
#include "errors.h"
#include "crc32.h"
#include "ltdefs.h"
#include "tcp.h"

#include <iostream>
#include <netinet/in.h>
#include <list>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>



Peer::Peer(const char myIP[], const char hubIP[],
           std::ostream& out_s, std::ostream& log_s):
           out(out_s), log(log_s), hub(hubIP, HUB_PORT), peerServer(PEER_PORT), closing(false){
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
    if(status < 0 && errno != EISCONN) sysError("ERROR connecting to hub");
    info("Connected to hub", this->log);
    // Request torrent
    status = this->getTorrent();
    if(status < 0) error("Could not get torrent", this->log);
    do{
        // Set packet to 0
        memset((char *) &pkt, 0, sizeof(pkt));
        status = this->hub.read((char *) &pkt.ph, sizeof(pkt.ph), true);
        if(status < 0){
            sysError("Could not read packet from hub", this->log);
            break;
        }
        if(status == 0) continue;
        isFIN = pkt.ph.type == FIN && pkt.ph.size == 0;
    }while(!isFIN);
}

int Peer::getTorrent(){
    int status;
    Packet pkt;
    ChunkHeader chunkHeader;
    int numCHs;
    // Check if connected
    if(!this->hub.isConn()){
        error("Hub not connected", this->log);
        return -1;
    }
    // Set packet to 0
    memset((char *) &pkt, 0, sizeof(pkt));
    // Form torrent request
    pkt.ph.type = TRRNT_REQ;
    pkt.ph.size = 0;
    // Write torrent request to hub
    status = this->hub.write((char *) &pkt.ph, sizeof(pkt.ph));
    if(status < 0){
        sysError("Could not write torrent request", this->log);
        return -1;
    }
    // Read torrent response from hub
    status = this->hub.read((char *) &pkt.ph, sizeof(pkt.ph));
    if(status < 0){
        sysError("Could not read torrent request", this->log);
        return -1;
    }
    // Read torrent response from hub
    status = this->hub.read((char *) pkt.payload, pkt.ph.size);
    if(status < 0){
        sysError("Could not read torrent request", this->log);
        return -1;
    }
    // Determine number of chunk headers
    numCHs = pkt.ph.size / sizeof(ChunkHeader);
    int n = 0;  // Chunk header location in payload
    for(int i=0; i < numCHs; i++){
        // Calculate chunk header location in payload
        n = i * sizeof(ChunkHeader);
        // Set temp chunk header to 0
        memset(&chunkHeader, 0 , sizeof(chunkHeader));
        // Copy memory to temp chunk header
        memcpy(&chunkHeader, &pkt.payload[n], sizeof(ChunkHeader));
        // TODO: Remove debugging statement
        this->log << chunkHeader.index << ' ' << chunkHeader.size << ' ' << chunkHeader.hash << std::endl;
        // Push chunk header onto torrent linked list
        this->torrent.push_back(chunkHeader);
    }
    return 0;
}

int Peer::sendFIN(){
    int status;
    // Form FIN packet
    PacketHeader finHdr = {FIN, 0};
    // Write FIN packet to hub
    info("FIN sending", this->log);
    status = this->hub.write((char *) &finHdr, sizeof(finHdr));
    if(status < 0){
        sysError("Couldn't write FIN to hub", this->log);
        return -1;
    }
    info("FIN sent", this->log);
    return 0;
}

void Peer::close(){
    int status;
    this->log << std::endl;
    // Send FIN to hub
    status = this->sendFIN();
    if(status < 0) error("Error sending FIN", this->log);
    // Notify other threads of closing
    this->closing = true;
    // Close socket connection to hub
    status = this->hub.close();
    if(status < 0){
        sysError("Couldn't close hub connection", this->log);
    }
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
