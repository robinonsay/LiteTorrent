#ifndef LTDEFS_H
#define LTDEFS_H

#include <netinet/in.h>

#define HUB_PORT 6969
#define PEER_PORT 6881
#define CHUNK_SIZE 524288  // 512kB
#define BACKLOG_QUEUE_SIZE 20

typedef struct sockaddr_in sockaddr_in;

enum reqTypes{
    TFileReq=0,
    TFileResp=1,
    ChunkInqReq=2,
    ChunkInqResp=3,
    ChunkReq=4,
    ChunkResp=5,
    FIN=6
};

typedef struct PacketHeader {
    uint32_t type;
    uint32_t size;
} PacketHeader;

typedef struct Packet{
    PacketHeader ph;
    char payload[CHUNK_SIZE];
} Packet;

typedef struct ChunkHeader{
    uint32_t index;
    uint32_t size;
    uint32_t hash;
} ChunkHeader;

typedef struct Chunk{
    ChunkHeader ch;
    char payload[CHUNK_SIZE];
} Chunk;


#endif

