#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdexcept>

void sysError(const char msg[], std::ostream& stream){
    stream << BOLDRED << msg << RESET << std::endl;
    perror("ERROR");
    throw std::runtime_error(msg);
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
