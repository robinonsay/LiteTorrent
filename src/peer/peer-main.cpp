#include "errors.h"
#include "peer/peer.h"
#include "btldefs.h"
#include "crc32.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <list>
#include <map>

#define MIN_ARGS 7

std::ifstream *inFile = NULL;
std::ofstream *outFile = NULL;
std::ofstream *logFile = NULL;
Peer *peer = NULL;

void signalHandler(int sigNum){
    if(peer != NULL) peer->closePeer();
    if(inFile != NULL){
        inFile->close();
        delete inFile;
    }
    if(outFile != NULL){
        outFile->close();
        delete outFile;
    }
    if(logFile != NULL){
        logFile->close();
        delete logFile;
    }
    exit(sigNum);
}

int main(int argc, char *argv[]){
    std::list<uint32_t> ocIndicies;
    std::map<uint32_t, CHUNK> owndChunks;
    std::ifstream owndChunksFile (argv[4]);
    signal(SIGINT, signalHandler);
    if(argc != MIN_ARGS){
        info(&std::cout, "Usage:\n./peer <my-ip> <tracker-ip> <input-file> <owned-chunks> <output-file> <logFile>");
        error(&std::cerr, "Invalid Arguments");
    }
    inFile = new std::ifstream(argv[3]);
    outFile = new std::ofstream(argv[5]);
    logFile = new std::ofstream(argv[6]);
    if(!inFile->is_open()){
        error(&std::cerr, "Input file does not exist");
    }
    if(!owndChunksFile.is_open()){
        error(&std::cerr, "Owned Chunks file does not exist");
    }
    if(!outFile->is_open()){
        error(&std::cerr, "Output file could not be opened. Check path");
    }
    if(!logFile->is_open()){
        error(&std::cerr, "Log file could not be opened. Check path");
    }
    uint32_t index;
    while(owndChunksFile >> index){
        printf("index: %d\n", index);
        ocIndicies.push_back(index);
    }
    owndChunksFile.close();
    ocIndicies.sort();
    CHUNK chunk;
    int i = ocIndicies.front();
    ocIndicies.pop_front();
    int pos = i * CHUNK_SIZE;
    inFile->seekg(0, inFile->end);
    int fileLen = inFile->tellg();
    inFile->seekg(0, inFile->beg);
    while(inFile->good() && pos < fileLen){
        memset((char *) &chunk, 0, sizeof(chunk));
        inFile->seekg(pos);
        inFile->read((char *) &chunk.payload, sizeof(chunk.payload));
        if(inFile->fail() && !inFile->eof())
            error(&std::cerr, "ERROR could not read input file");
        int bytesRead = inFile->gcount();
        chunk.ch.index = i;
        chunk.ch.hash = crc32(chunk.payload, CHUNK_SIZE);
        chunk.ch.length = bytesRead;
        printf("%u %u\n", i, chunk.ch.hash);
        owndChunks[chunk.ch.hash] = chunk;
        if(ocIndicies.empty()) break;
        i = ocIndicies.front();
        ocIndicies.pop_front();
        pos = i * CHUNK_SIZE;
    }
    peer = new Peer(argv[1], argv[2], &owndChunks, outFile, logFile);
    peer->run();
    peer->closePeer();
    inFile->close();
    outFile->close();
    logFile->close();
    delete peer;
    delete outFile;
    delete logFile;
    return 0;
}
