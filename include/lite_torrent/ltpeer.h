#ifndef PEER_WRPR_H
#define PEER_WRPR_H

#include "lite_torrent/ltdefs.h"

#include <list>
#include <map>
#include <string>

typedef std::list<ChunkHeader> ChunkHList;

class LtPeer{
private:
    ChunkHList chunkHdrs;
    std::string ipv4Str;
public:
    LtPeer(std::string ipv4Str);
    LtPeer();
    ~LtPeer();
    ChunkHeader& back();
    ChunkHeader& front();
    ChunkHList::iterator begin();
    ChunkHList::iterator end();
    size_t size();
    void sort();
    std::string& getIPv4Str();
    void addChunk(ChunkHeader ch);
};

typedef std::map<std::string, LtPeer> PeerMap;

#endif
