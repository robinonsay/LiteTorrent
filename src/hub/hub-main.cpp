#include "hub/hub.h"
#include "errors.h"
#include "argparse.h"

#include <fstream>
#include <iostream>
#include <map>
#include <list>
#include <stdio.h>
#include <csignal>

Hub *hub = NULL;
std::ifstream *inFile = NULL;
std::ofstream *logFile = NULL;

void signalHandler(int signum){
    if(hub != NULL) hub->close();
    if(inFile != NULL) inFile->close();
    if(logFile != NULL) logFile->close();
    exit(signum);
}

int main(int argc, char *argv[]){
    ArgParse argParser (argc, argv, {"input-file", "log-file"});
    ArgMap args = argParser.parseArgs();
    signal(SIGINT, signalHandler);
    inFile = new std::ifstream(args["input-file"]);
    logFile = new std::ofstream(args["log-file"]);
    if(!inFile->is_open())
        fatalError(std::cerr, "Could not open input file. Check path");
    if(!logFile->is_open())
        fatalError(std::cerr, "Could not open log file. Check path");
    hub = new Hub(*inFile);
    hub->run();
    return 0;
}

