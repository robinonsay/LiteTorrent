#include "mutex/shared_mutex.h"

#include <atomic>

SharedMutex::SharedMutex(): n(0){

}

void SharedMutex::lockRead(){
    this->n++;
    if(this->n > 0) this->wMtx.lock();
}
void SharedMutex::lockWrite(){
    this->wMtx.lock();
}
void SharedMutex::unlockRead(){
    this->n--;
    if(this->n == 0) this->wMtx.unlock();
}
void SharedMutex::unlockWrite(){
    this->wMtx.unlock();
}
