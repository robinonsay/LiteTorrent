#ifndef ERRORS_H
#define ERRORS_H

#include <iostream>
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

/**
* Logs system error to stream and prints message to console
* @param stream Stream to log error to; defaults to std::cerr
* @param msg Error message
*/
void sysError(const char msg[], std::ostream& stream=std::cerr);

/**
* Logs error to stream
* @param stream Stream to log error to; defaults to std::cerr
* @param msg Error message
*/
void error(const char msg[], std::ostream& stream=std::cerr);

/**
* Logs warning to stream
* @param stream Stream to log warning to; defaults to std::cout
* @param msg Warning message
*/
void warning(const char msg[], std::ostream& stream=std::cout);

/**
* Logs info to stream
* @param stream Stream to log info to; defaults to std::cout
* @param msg Error message
*/
void info(const char msg[], std::ostream& stream=std::cout);

/**
* Prints info to stream
* @param stream Stream to log info to; defaults to std::cout
* @param msg Error message
*/
void print(const char msg[], std::ostream& stream=std::cout);

namespace tcp{
    /** Exception for TCP related errors */
    class error: public std::runtime_error {
        public:
          explicit error(const std::string& what_arg);
          explicit error(const char* what_arg);
    };

    /** Exception for TCP related system errors */
    class sys_error: public std::runtime_error {
        public:
          explicit sys_error(const std::string& what_arg);
          explicit sys_error(const char* what_arg);
    };
}

#endif
