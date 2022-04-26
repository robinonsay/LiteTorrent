#include "errors.h"
#include <stdio.h>
#include <stdlib.h>

void errInvalidArgs(){
    fprintf(stderr, "Error: invalid arguments\n");
    exit(1);
}

void errInvalidPort(){
    fprintf(stderr, "Error: port number not in range\n");
    exit(1);
}

void errInvalidTime(){
    fprintf(stderr, "Error: time argument must be greater than 0\n");
    exit(1);
}

void sysError(const char msg[]){
    perror(msg);
    exit(1);
}

