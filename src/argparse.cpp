#include "argparse.h"
#include "errors.h"

#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <stdio.h>
#include <map>
#include <list>
#include <initializer_list>

ArgParse::ArgParse(int argc, char *argv[], std::initializer_list<std::string> il){
    // Set order to initializer list
    this->order = il;
    // Argument list iterator
    ArgList::iterator it;
    // String stream for help message
    std::ostringstream oss;
    // Build help message
    oss << "Usage:" << std::endl;
    oss << argv[0];
    for(it = this->order.begin(); it != this->order.end(); ++it){
        oss << " <" << *it << ">";
    }
    // Set help message
    this->helpMsg = oss.str();
    this->argc = argc;
    this->argv = argv;
}

ArgParse::~ArgParse(){
}

std::string ArgParse::getHelp(){
    // return help message
    return this->helpMsg;
}

ArgMap& ArgParse::parseArgs(){
    ArgList::iterator it;
    int i = 1;
    // Check if number of arguments was passed
    if(this->order.size() != (size_t) this->argc - 1){
        error(std::cerr, "Invalid Argument(s)");
        info(std::cout, this->helpMsg.c_str());
        exit(1);
    }
    // Set arguments to map
    for(it = this->order.begin(); it != this->order.end() && i < this->argc; ++it){
        this->args[*it] = this->argv[i];
        i++;
    }
    // Return map by reference
    return this->args;
}

