#include "errors.h"
#include "peer/peer.h"
#include "ltdefs.h"
#include "crc32.h"
#include "argparse.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <list>
#include <map>

#define MIN_ARGS 7

std::ofstream *outFile = NULL;
std::ofstream *logFile = NULL;
Peer *peer = NULL;

void signalHandler(int sigNum){
    if(peer != NULL){
        peer->close();
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
    ArgParse argParser (argc, argv, {"myIP", "hubIP", "output-file", "log-file"});
    ArgMap args = argParser.parseArgs();
    signal(SIGINT, signalHandler);
    outFile = new std::ofstream(args["output-file"]);
    logFile = new std::ofstream(args["log-file"]);
    if(!outFile->is_open()){
        error("Could not open input file. Check path");
        exit(1);
    }
    if(!logFile->is_open()){
        error("Could not open log file. Check path");
        exit(1);
    }
    peer = new Peer(args["myIP"].c_str(), args["hubIP"].c_str(), *outFile);
    peer->run();
    outFile->close();
    logFile->close();
    return 0;
}
