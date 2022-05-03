#ifndef ERRORS_H
#define ERRORS_H

#include <iostream>

void sysError(const char msg[]);
void error(std::ostream *stream, const char msg[]);

#endif
