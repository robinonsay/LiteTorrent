#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdexcept>

void sysError(const char msg[]){
    perror(msg);
    throw std::runtime_error(msg);
}

void fatalSysError(const char msg[]){
    perror(msg);
    exit(1);
}

void fatalError(std::ostream& stream, const char msg[]){
    stream << RED << msg << RESET << std::endl;
    exit(1);
}

void error(std::ostream& stream, const char msg[]){
    stream << RED << msg << RESET << std::endl;
}

void warning(std::ostream& stream, const char msg[]){
    stream << YELLOW << msg << RESET << std::endl;
}

void info(std::ostream& stream, const char msg[]){
    stream << CYAN << msg << RESET << std::endl;
}

