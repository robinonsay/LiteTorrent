#include "lite_torrent/mrsw_mutex.h"

#include <atomic>

MRSWMutex::MRSWMutex(){

}

void MRSWMutex::lockRead(){
    this->rMtx.lock();
    this->n++;
    if(this->n == 1) this->wMtx.lock();
    this->rMtx.unlock();
}
void MRSWMutex::lockWrite(){
    this->wMtx.lock();
}
void MRSWMutex::unlockRead(){
    this->rMtx.lock();
    this->n--;
    if(this->n == 0) this->wMtx.unlock();
    this->rMtx.unlock();
}
void MRSWMutex::unlockWrite(){
    this->wMtx.unlock();
}
