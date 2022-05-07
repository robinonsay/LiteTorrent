#include "lite_torrent/ltdefs.h"
#include <sstream>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>

std::string addrIPv4ToString(sockaddr_in *addr){
    std::ostringstream addrStream;
    addrStream << inet_ntoa(addr->sin_addr) << ":" << ntohs(addr->sin_port);
    return addrStream.str();
}
