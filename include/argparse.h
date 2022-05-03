#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>
#include <map>
#include <list>

typedef std::map<std::string, std::string> ArgMap;
typedef std::list<std::string> ArgList;

class ArgParse{
    public:
        ArgParse(int argc, char *argv[]);
        ~ArgParse();
        std::string getHelp();
        ArgMap& parseArgs();
    private:
        std::string helpMsg;
        ArgMap args;
        ArgList order;
        int argc;
        char **argv;
};

#endif

