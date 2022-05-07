#include "peer/ltpeer.h"
#include "ltdefs.h"

LtPeer::LtPeer(std::string ipv4Str){
    this->ipv4Str = ipv4Str;
}

LtPeer::LtPeer(){
}

LtPeer::~LtPeer(){

}

ChunkHeader& LtPeer::back(){
    return this->chunkHdrs.back();
}

ChunkHeader& LtPeer::front(){
    return this->chunkHdrs.front();
}

ChunkHList::iterator LtPeer::begin(){
    return this->chunkHdrs.begin();
}

ChunkHList::iterator LtPeer::end(){
    return this->chunkHdrs.end();
}

std::string& LtPeer::getIPv4Str(){
    return this->ipv4Str;
}

void LtPeer::addChunk(ChunkHeader ch){
    this->chunkHdrs.push_back(ch);
}

size_t LtPeer::size(){
    return this->chunkHdrs.size();
}

void LtPeer::sort(){
    this->chunkHdrs.sort([](ChunkHeader lhs, ChunkHeader rhs){
                             return lhs.index < rhs.index;
                         });
}
