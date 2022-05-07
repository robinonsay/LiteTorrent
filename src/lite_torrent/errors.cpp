#include "lite_torrent/errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdexcept>

void sysError(const char msg[], std::ostream& stream){
    stream << BOLDRED << msg << RESET << std::endl;
    perror("ERROR");
}


void error(const char msg[], std::ostream& stream){
    stream << BOLDRED << msg << RESET << std::endl;
}

void warning(const char msg[], std::ostream& stream){
    stream << BOLDYELLOW << msg << RESET << std::endl;
}

void info(const char msg[], std::ostream& stream){
    stream << BOLDCYAN << msg << RESET << std::endl;
}

void print(const char msg[], std::ostream& stream){
    stream << msg << std::endl;
}

tcp::error::error(const std::string& what_arg): std::runtime_error(what_arg){

}

tcp::error::error(const char* what_arg): std::runtime_error(what_arg){

}

tcp::sys_error::sys_error(const std::string& what_arg): std::runtime_error(what_arg){

}

tcp::sys_error::sys_error(const char* what_arg): std::runtime_error(what_arg){

}
