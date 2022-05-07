#ifndef HUB_H
#define HUB_H

#include "lite_torrent/ltdefs.h"
#include "lite_torrent/mrsw_mutex.h"
#include "lite_torrent/tcp.h"
#include "lite_torrent/ltpeer.h"

#include <atomic>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <thread>

typedef std::list<std::thread> ThreadList;
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
    void close(bool interrupt=false);

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

    /** Map IPv4 strs of peers to LtPeers */
    PeerMap peerMap;

    /** AddrChunkMap mutex */
    MRSWMutex pmMtx;

    /** Map of IPv4 peer address strings to sockaddr_in structures */
    AddrMap peerAddrMap;

    /** Update Peers mutex */
    std::mutex upMtx;

    /** Flag indicating closing of the Hub */
    std::atomic<bool> closing;

    /** Flag indicating closing of the Hub */
    std::atomic<size_t> peersInMap;

    /** Connection Handler */
    void peerConnHandler(std::string peerIPv4);

    /** Updates peers of new peer on network */
    void updatePeers();
};
#endif
