#ifndef __PACKET_HEADER_H__
#define __PACKET_HEADER_H__

struct PacketHeader
{
    unsigned int type;     // 0 = request torrent, 1 = sending torrent file, 2 = request chunk list, 3 = chunk request list response, 4 = get chunk, 5 = get chunk response
    unsigned int length;   // Length of data, not including the header
};

#endif