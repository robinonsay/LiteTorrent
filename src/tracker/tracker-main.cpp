#include "errors.h"
#include "tracker/tracker.h"

#include <fstream>
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
    if(argc != MIN_ARGS){
        printf("Invalid Arguments\nUsage:\n./tracker <peers-list> <input-file> <torrent-file> <logFile>\n");
        exit(1);
    }
    logFile = new std::ofstream(argv[4]);
    if(logFile->is_open()){
        trackerServer = new Tracker(argv[1], argv[3], argv[2], logFile);
        trackerServer->run();
        trackerServer->closeTracker();
    }else{
        printf("Log file could not be opened. Check path\n");
    }
    logFile->close();
    delete trackerServer;
    delete logFile;
    return 0;
}

