#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>
#include <map>
#include <list>
#include <initializer_list>

typedef std::map<std::string, std::string> ArgMap;
typedef std::list<std::string> ArgList;

class ArgParse{
    public:
        /**
        * ArgParse parses the given command line arguments given that all command line arguments provided are required
        * @param argc argument count
        * @param argv argument list
        * @param il initializer list for required arguments
        */
        ArgParse(int argc, char *argv[], std::initializer_list<std::string> il);
        /** ArgParse destructor
        */
        ~ArgParse();
        /**
        * Gets help message
        * @return help message
        */
        std::string getHelp();
        /**
        * Parse command line arguments
        * @return Mapping of required arguments to command line arguments
        */
        ArgMap& parseArgs();
    private:
        /**
        * Help message
        */
        std::string helpMsg;
        /**
        * Argument map
        */
        ArgMap args;
        /**
        * Required args list
        */
        ArgList order;
        /**
        * Argument count
        */
        int argc;
        /**
        * Argument list
        */
        char **argv;
};

#endif

