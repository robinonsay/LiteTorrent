#include "errors.h"

#include <fstream>
#include <stdio.h>
#include <string.h>

#define MIN_ARGS 5

int main(int argc, char *argv[]){
//    struct sockaddr_in server_addr;
//    socklen_t addrlen = sizeof(server_addr);
//    int status, sockfd, receiverPort; 
//    char *receiverIP;
    std::ifstream *peersList;
    std::ifstream *inFile;
    std::ofstream *tFile;
    std::ofstream *log;
    if(argc != MIN_ARGS){
        printf("Invalid Arguments\nUsage:\n./tracker <peers-list> <input-file> <torrent-file> <log>\n");
        exit(1);
    }
//    // create UDP socket
//    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//    if(sockfd < 0) sysError("ERROR creating socket");
//    memset((char *) &server_addr, 0, sizeof(server_addr));
//    server_addr.sin_family = AF_INET;
//    server_addr.sin_port = htons(receiverPort);    // specify port to connect to
//    status = inet_aton(receiverIP, &server_addr.sin_addr);
//    if(status == 0){
//        fprintf(stderr, "ERROR invalid IP address\n");
//        exit(1);
//    }
    peersList = new std::ifstream(argv[1]);
    inFile = new std::ifstream(argv[2], std::ifstream::binary);
    tFile = new std::ofstream(argv[3]);
    log = new std::ofstream(argv[4], std::ofstream::app);
    if(!peersList->is_open()) printf("Peers list file does not exist\n");
    if(!inFile->is_open()) printf("Input file does not exist\n");
    if(!tFile->is_open()) printf("Torrent file could not be opened. Check path\n");
    if(!log->is_open()) printf("Log file could not be opened. Check path\n");
    peersList->close();
    inFile->close();
    tFile->close();
    log->close();
    delete peersList;
    delete inFile;
    delete tFile;
    delete log;
    return 0;
}

