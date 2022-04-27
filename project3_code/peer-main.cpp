#include "errors.h"

#include <fstream>
#include <stdio.h>
#include <string.h>

#define MIN_ARGS 7

int main(int argc, char *argv[]){
//    struct sockaddr_in server_addr;
//    socklen_t addrlen = sizeof(server_addr);
//    int status, sockfd, receiverPort; 
    char *myIP;
    char *trackerIP;
    std::string inFilePath;
    std::ifstream *owndChunksFile;
    std::ofstream *outFile;
    std::ofstream *log;
    if(argc != MIN_ARGS){
        printf("Invalid Arguments\nUsage:\n./peer <my-ip> <tracker-ip> <input-file> <owned-chunks> <output-file> <log>\n");
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
    myIP = argv[1];
    trackerIP = argv[2];
    inFilePath = std::string(argv[3]);
    owndChunksFile = new std::ifstream(argv[4]);
    outFile = new std::ofstream(argv[5]);
    log = new std::ofstream(argv[6], std::ofstream::app);
    if(!owndChunksFile->is_open()) printf("Owned Chunks file does not exist\n");
    if(!outFile->is_open()) printf("Output file could not be opened. Check path\n");
    if(!log->is_open()) printf("Log file could not be opened. Check path\n");
    owndChunksFile->close();
    outFile->close();
    log->close();
    delete owndChunksFile;
    delete outFile;
    delete log;
    return 0;
}

