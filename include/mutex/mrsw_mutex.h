#ifndef SHARED_MUTEX_H
#define SHARED_MUTEX_H

#include <mutex>

class MRSWMutex{
public:
    MRSWMutex();
    void lockRead();
    void lockWrite();
    void unlockRead();
    void unlockWrite();
private:
    int n = 0;
    std::mutex wMtx;
    std::mutex rMtx;
};

#endif
