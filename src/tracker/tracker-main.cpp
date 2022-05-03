#include "errors.h"
#include "tracker/tracker.h"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <csignal>

#define MIN_ARGS 5

Tracker *trackerServer = NULL;
std::ofstream *logFile = NULL;

void signalHandler(int signum){
    if(trackerServer != NULL) trackerServer->closeTracker();
    if(logFile != NULL){
        logFile->close();
        delete logFile;
    }
    exit(signum);
}

int main(int argc, char *argv[]){
    signal(SIGINT, signalHandler);
    if(argc != MIN_ARGS) {
        info(&std::cout, "Usage:\n./tracker <peers-list> <input-file> <torrent-file> <logFile>");
        error(&std::cerr, "Invalid Arguments");
    }
    logFile = new std::ofstream(argv[4]);
    if(logFile->is_open()){
        trackerServer = new Tracker(argv[1], argv[3], argv[2], logFile);
        trackerServer->run();
        trackerServer->closeTracker();
    }else{
        logFile->close();
        error(&std::cerr, "Log file could not be opened. Check path");
    }
    logFile->close();
    delete trackerServer;
    delete logFile;
    return 0;
}

