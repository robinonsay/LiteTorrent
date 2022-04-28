#include "errors.h"
#include "tracker.h"

#include <fstream>
#include <stdio.h>
#include <string.h>
#include <csignal>

#define MIN_ARGS 5

Tracker *trackerServer = NULL;
std::ofstream *log = NULL;

void signalHandler(int signum){
    if(trackerServer != NULL) delete trackerServer;
    if(log != NULL){
        log->close();
        delete log;
    }
    exit(signum);
}

int main(int argc, char *argv[]){
//    struct sockaddr_in server_addr;
//    socklen_t addrlen = sizeof(server_addr);
//    int status, sockfd, receiverPort; 
//    char *receiverIP;
    signal(SIGINT, signalHandler);
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
    log = new std::ofstream(argv[4], std::ofstream::app);
    if(log->is_open()){
        trackerServer = new Tracker(argv[1], argv[3], argv[2], log);
    }else{
        printf("Log file could not be opened. Check path\n");
    }
    log->close();
    delete trackerServer;
    delete log;
    return 0;
}

