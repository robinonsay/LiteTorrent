#ifndef PEER_WRPR_H
#define PEER_WRPR_H

#include "ltpeer.h"

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
    ~LtPeer();
    ChunkHeader& back();
    ChunkHeader& front();
    ChunkHList::iterator& begin();
    ChunkHList::iterator& end();
    void sort();
    std::string& getIPv4Str();
    void addChunk(ChunkHeader ch);
};
#endif
