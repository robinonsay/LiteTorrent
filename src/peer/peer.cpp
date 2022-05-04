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



Peer::Peer(const char myIP[], const char hubIP[],
           std::ostream& out_s, std::ostream& log_s):
           out(out_s), log(log_s), hub(hubIP, HUB_PORT), peerServer(PEER_PORT), closing(false){
    this->myIP = myIP;
}

Peer::~Peer(){
}

void Peer::run(){
    int status;
    bool isFIN = false;
    Packet pkt;
    memset((char *) &pkt, 0, sizeof(pkt));
    status = this->hub.connect();
    if(status < 0 && errno != EISCONN) sysError("ERROR connecting to hub");
    this->log << "Connected to hub" << std::endl;
    this->threads.push_back(std::thread(&Peer::server, this));
    do{
        status = this->hub.read((char *) &pkt.ph, sizeof(pkt.ph)); 
        if(status < 0) sysError("ERROR reading hub packet");
        switch(pkt.ph.type){
            case FIN:
                isFIN = true;
        }
    }while(!isFIN);
    this->log << "Recieved FIN" << std::endl;
}

void Peer::close(){
    int status;
    PacketHeader finPkt = {FIN, 0};
    this->closing = true;
    status = this->hub.write((char *) &finPkt, sizeof(finPkt));
    if(status < 0) sysError("ERROR writing FIN to hub");
    status = this->hub.close();
    if(status < 0) sysError("ERROR closing hub connection");
    ThreadList::iterator thi;
    for(thi=this->threads.begin(); thi != this->threads.end(); ++thi){
        thi->join();
    }
}

void Peer::server(){
    while(!this->closing){
        sleep(1);
    }
    this->log << "Closing peer server" << std::endl;
}

void Peer::connHandler(int peerSockfd, sockaddr_in peerAddr){
}

