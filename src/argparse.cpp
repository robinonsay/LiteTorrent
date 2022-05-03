#include "argparse.h"
#include "errors.h"

#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <stdio.h>
#include <map>
#include <list>

ArgParse::ArgParse(int argc, char *argv[]){
    this->order = {"input-file", "log-file"};
    ArgList::iterator it;
    std::ostringstream oss;
    oss << "Usage:" << std::endl;
    oss << argv[0];
    for(it = this->order.begin(); it != this->order.end(); ++it){
        oss << " <" << *it << ">";
    }
    this->helpMsg = oss.str();
    this->argc = argc;
    this->argv = argv;
}

ArgParse::~ArgParse(){
}

std::string ArgParse::getHelp(){
    return this->helpMsg;
}

ArgMap& ArgParse::parseArgs(){
    ArgList::iterator it;
    int i = 1;
    if(this->order.size() != (size_t) this->argc - 1){
        error(std::cerr, "Invalid Argument(s)");
        info(std::cout, this->helpMsg.c_str());
        exit(1);
    }
    for(it = this->order.begin(); it != this->order.end() && i < this->argc; ++it){
        this->args[*it] = this->argv[i];
        i++;
    }
    return this->args;
}

