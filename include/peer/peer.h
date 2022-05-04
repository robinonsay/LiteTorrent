#ifndef PEER_H
#define PEER_H

#include "ltdefs.h"
#include "tcp.h"

#include <iostream>
#include <netinet/in.h>
#include <list>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>

#define PEER_TIMEOUT_ms 30000

typedef std::list<std::thread> ThreadList;

class Peer{
    private:
        std::ostream& out;
        std::ostream& log;
        TCPClient hub;
        TCPServer peerServer;
        const char *myIP;
        ThreadList threads;
        std::mutex mutex;
        std::atomic<bool> closing;
        void server();
        void connHandler(int peerSockfd, sockaddr_in peerAddr);
    public:
        Peer(const char myIP[], const char hubIP[], std::ostream& out_s, std::ostream& log_s=std::cout);
        ~Peer();
        void run();
        void close();
};

#endif

