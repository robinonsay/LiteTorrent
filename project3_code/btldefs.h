#ifndef BTLDEFS_H
#define BTLDEFS_H

#include "PacketHeader.h"

#include <netinet/in.h>

#define TRACKER_PORT 6969
#define PEER_PORT 6881
#define CHUNK_SIZE 512000
#define BACKLOG_QUEUE_SIZE 20

typedef struct sockaddr_in IP_ADDR;

enum types{
    TrrntFileReq=0,
    TrrntFileResp=1,
    ChunkInqReq=2,
    ChunkInqResp=3,
    ChunkReq=4,
    ChunkResp=5
};

typedef struct PacketHeader PACKET_HEADER;

typedef struct Packet{
    PACKET_HEADER ph;
    char payload[CHUNK_SIZE];
} PACKET;

typedef struct chunkHeader{
    unsigned int index;
    uint32_t hash;
} CHUNK_H;

 typedef struct chunk{
    CHUNK_H ch;
    char payload[CHUNK_SIZE];
 } CHUNK;

#endif

