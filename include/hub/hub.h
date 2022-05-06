#ifndef HUB_H
#define HUB_H

#include "ltdefs.h"
#include "tcp.h"

#include <map>
#include <list>
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>

typedef std::list<std::thread> ThreadList;
typedef std::map<std::string, std::list<ChunkHeader>> AddrChunkMap;
typedef std::map<std::string, sockaddr_in> AddrMap;


#define TIMEOUT 30000  // in ms
#define BACKLOG_SIZE 10

/**
* Hub is the central server for LiteTorrent.
* It sends peers the torrent list and updates all peers
* when a new peer has joined the network
*/
class Hub{
    public:

        /**
        * Creates Hub
        * @param in_s The input stream to share with peers (most likely a file)
        * @param log_s The log file stream; defaults to std::cout*/
        Hub(std::istream& in_s=std::cin, std::ostream& log_s=std::cout);
        ~Hub();

        /**
        * Closes the Hub
        */
        void close();

        /** Runs the Hub */
        void run();
    private:

        /** The input stream for chunking */
        std::istream& in;

        /** The log stream */
        std::ostream& log;

        /** The torrent packet */
        Packet torrentPkt;

        /** The TCP server */
        TCPServer server;

        /** The thread pool */
        ThreadList threads;

        /** Map of peer address to chunks it owns */
        AddrChunkMap peerMap;

        /** Map of IPv4 peer address strings to sockaddr_in structures */
        AddrMap peerAddrMap;

        /** Flag indicating closing of the Hub */
        std::atomic<bool> closing;

        /** Connection Handler */
        void peerConnHandler(std::string peerIPv4);

        /** Updates peers of new peer on network */
        void updatePeers();
};
#endif
