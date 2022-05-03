#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

void sysError(const char msg[]){
    perror(msg);
    exit(1);
}

void error(std::ostream *stream, const char msg[]){
    (*stream) << msg << std::endl;
    exit(1);
}

