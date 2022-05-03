#include "hub/hub.h"
#include "errors.h"
#include "argparse.h"

#include <iostream>
#include <map>
#include <list>
#include <stdio.h>

int main(int argc, char *argv[]){
    ArgParse argParser (argc, argv);
    ArgMap args = argParser.parseArgs();
    return 0;
}

