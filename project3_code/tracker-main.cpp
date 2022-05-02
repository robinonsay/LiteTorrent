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
    if(trackerServer != NULL) trackerServer->closeTracker();
    if(log != NULL){
        log->close();
        delete log;
    }
    exit(signum);
}

int main(int argc, char *argv[]){
    signal(SIGINT, signalHandler);
    if(argc != MIN_ARGS){
        printf("Invalid Arguments\nUsage:\n./tracker <peers-list> <input-file> <torrent-file> <log>\n");
        exit(1);
    }
    log = new std::ofstream(argv[4]);
    if(log->is_open()){
        trackerServer = new Tracker(argv[1], argv[3], argv[2], log);
        trackerServer->run();
        trackerServer->closeTracker();
    }else{
        printf("Log file could not be opened. Check path\n");
    }
    log->close();
    delete trackerServer;
    delete log;
    return 0;
}

