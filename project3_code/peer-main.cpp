#include "errors.h"
#include "peer.h"

#include <fstream>
#include <stdio.h>
#include <string.h>

#define MIN_ARGS 7

std::ifstream *owndChunksFile = NULL;
std::ofstream *outFile = NULL;
std::ofstream *log = NULL;
Peer *peer = NULL;

void signalHandler(int sigNum){
    delete peer;
    if(owndChunksFile != NULL){
        owndChunksFile->close();
        delete owndChunksFile;
    }
    if(outFile != NULL){
        outFile->close();
        delete outFile;
    }
    if(log != NULL){
        log->close();
        delete log;
    }
    exit(sigNum);
}

int main(int argc, char *argv[]){
    char *myIP;
    char *trackerIP;
    std::string inFilePath;
    bool filesOpnd = true;
    signal(SIGINT, signalHandler);
    if(argc != MIN_ARGS){
        printf("Invalid Arguments\nUsage:\n./peer <my-ip> <tracker-ip> <input-file> <owned-chunks> <output-file> <log>\n");
        exit(1);
    }
    myIP = argv[1];
    trackerIP = argv[2];
    inFilePath = std::string(argv[3]);
    owndChunksFile = new std::ifstream(argv[4]);
    outFile = new std::ofstream(argv[5]);
    log = new std::ofstream(argv[6], std::ofstream::app);
    if(!owndChunksFile->is_open()){
        printf("Owned Chunks file does not exist\n");
        filesOpnd = false;
    }
    if(!outFile->is_open()){
        printf("Output file could not be opened. Check path\n");
        filesOpnd = false;
    }
    if(!log->is_open()){
        printf("Log file could not be opened. Check path\n");
        filesOpnd = false;
    }
    if(filesOpnd){
        peer = new Peer(myIP, trackerIP, owndChunksFile, outFile, log);
        peer->run();
    }
    owndChunksFile->close();
    outFile->close();
    log->close();
    delete peer;
    delete owndChunksFile;
    delete outFile;
    delete log;
    return 0;
}

