#ifndef SHARED_MUTEX_H
#define SHARED_MUTEX_H

#include <atomic>
#include <mutex>

class SharedMutex{
public:
    SharedMutex();
    void lockRead();
    void lockWrite();
    void unlockRead();
    void unlockWrite();
private:
    std::atomic<int> n;
    std::mutex wMtx;
};

#endif
