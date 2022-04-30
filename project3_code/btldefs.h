#ifndef BTLDEFS_H
#define BTLDEFS_H

#include "PacketHeader.h"

#define TRACKER_PORT 6969
#define CHUNK_SIZE 512000
#define BACKLOG_QUEUE_SIZE 20

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

#endif

