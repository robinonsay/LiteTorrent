#include "errors.h"
#include "peer.h"
#include "btldefs.h"
#include "crc32.h"

#include <fstream>
#include <stdio.h>
#include <string.h>
#include <list>
#include <map>

#define MIN_ARGS 7

std::ifstream *inFile = NULL;
std::ofstream *outFile = NULL;
std::ofstream *log = NULL;
Peer *peer = NULL;

void signalHandler(int sigNum){
    delete peer;
    if(inFile != NULL){
        inFile->close();
        delete inFile;
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
    bool filesOpnd = true;
    std::list<unsigned int> ocIndicies;
    std::map<unsigned int, CHUNK> owndChunks;
    std::ifstream owndChunksFile (argv[4]);
    signal(SIGINT, signalHandler);
    if(argc != MIN_ARGS){
        printf("Invalid Arguments\nUsage:\n./peer <my-ip> <tracker-ip> <input-file> <owned-chunks> <output-file> <log>\n");
        exit(1);
    }
    inFile = new std::ifstream(argv[3]);
    outFile = new std::ofstream(argv[5]);
    log = new std::ofstream(argv[6], std::ofstream::app);
    if(!inFile->is_open()){
        printf("Input file does not exist\n");
        filesOpnd = false;
    }
    if(!owndChunksFile.is_open()){
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
        unsigned int index;
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
            inFile->seekg(pos);
            inFile->read((char *) &chunk.payload, sizeof(chunk.payload));
            if(inFile->fail() && !inFile->eof()){
                fprintf(stderr, "ERROR could not read input file\n");
                exit(1);
            }
            chunk.ch.index = i;
            chunk.ch.hash = crc32(chunk.payload, CHUNK_SIZE);
            printf("%u %u\n", i, chunk.ch.hash);
            if(ocIndicies.empty()) break;
            owndChunks[i] = chunk;
            i = ocIndicies.front();
            ocIndicies.pop_front();
            pos = i * CHUNK_SIZE;
        }
        peer = new Peer(argv[1], argv[2], &owndChunks, outFile, log);
        peer->run();
    }
    inFile->close();
    outFile->close();
    log->close();
    delete peer;
    delete outFile;
    delete log;
    return 0;
}

