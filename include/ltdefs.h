#ifndef LTDEFS_H
#define LTDEFS_H

#include "crypto.h"

#include <list>
#include <map>
#include <netinet/in.h>
#include <string>

#define HUB_PORT 6969
#define PEER_PORT 6881
#define CHUNK_SIZE 524288  // 512kB
#define BACKLOG_QUEUE_SIZE 20

typedef struct sockaddr_in sockaddr_in;

/**
* Types for packets
* Starts at 100 so it is not confused with NULL packets
*/
enum types{
    FIN=100,  /** FIN packet type*/
    UPDATE=200, /** Update packet type */
    TRRNT_REQ=300, /** Torrent request packet type */
    TRRNT_RESP=400  /** Torrent response packet type */
};

/** Packet header structure */
typedef struct PacketHeader {
    uint32_t type;  /** Packet type, see types */
    uint32_t size;  /** Packet payload size */
} PacketHeader;

/** Packet structure*/
typedef struct Packet{
    PacketHeader ph;  /** Packet header, see PacketHeader */
    char payload[CHUNK_SIZE];  /** Packet payload with size CHUNK_SIZE */
} Packet;

/** Chunk Header structure */
typedef struct ChunkHeader{
    uint32_t index;  /** Chunk index */
    uint32_t size;  /** Chunk size */
    HashSHA3_256 hash_s;  /** Chunk hash */
} ChunkHeader;

/** Chunk structure */
typedef struct Chunk{
    ChunkHeader ch;  /** Chunk header, see ChunkHeader */
    char payload[CHUNK_SIZE]; /** Chunk data, with size CHUNK_SIZE*/
} Chunk;

std::string addrIPv4ToString(sockaddr_in *addr);

#endif
