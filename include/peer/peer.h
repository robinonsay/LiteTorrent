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
typedef std::map<uint32_t, ChunkHeader> HashChunkHMap;
typedef std::map<uint32_t, ChunkHeader> IndexChunkHMap;
typedef std::list<ChunkHeader> ChunkHList;

class Peer{
    private:
        std::ostream& out;
        std::ostream& log;
        TCPClient hub;
        TCPServer peerServer;
        const char *myIP;
        ThreadList threads;
        std::atomic<bool> closing;
        ChunkHList torrent;
        int getTorrent();
        void server();
        void connHandler(sockaddr_in peerAddr);
    public:
        Peer(const char myIP[], const char hubIP[], std::ostream& out_s, std::ostream& log_s=std::cout);
        ~Peer();
        void run();
        void open();
        void close();
};

namespace p{
    /** Exception for TCP related errors */
    class error: public std::runtime_error {
        public:
          explicit error(const std::string& what_arg);
          explicit error(const char* what_arg);
    };

    /** Exception for TCP related system errors */
    class sys_error: public std::runtime_error {
        public:
          explicit sys_error(const std::string& what_arg);
          explicit sys_error(const char* what_arg);
    };
}

#endif
