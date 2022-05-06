#ifndef PEER_ERROR_H
#define PEER_ERROR_H

#include <stdexcept>

namespace p{
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
