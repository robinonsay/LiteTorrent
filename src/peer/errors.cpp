#include "peer/errors.h"

#include <stdexcept>

p::error::error(const std::string& what_arg): std::runtime_error(what_arg){

}

p::error::error(const char* what_arg): std::runtime_error(what_arg){

}

p::sys_error::sys_error(const std::string& what_arg): std::runtime_error(what_arg){

}

p::sys_error::sys_error(const char* what_arg): std::runtime_error(what_arg){

}
