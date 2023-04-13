#pragma once
#include <pthread.h>
#include <iostream>
using namespace std;

class Mutex
{
public:
    Mutex()
    {pthread_mutex_init(&_mutex,nullptr);}
    
    ~Mutex()
    {pthread_mutex_destroy(&_mutex);}
    
    void lock()
    {pthread_mutex_lock(&_mutex);}
    
    void unlock()
    {pthread_mutex_unlock(&_mutex);}
private:
    pthread_mutex_t _mutex;
};


class LockGuard
{
public:
    LockGuard(Mutex *mutex)
    {
        _mutex = mutex;
        _mutex->lock();

        cout << "加锁成功" << endl;
    }
    ~LockGuard()
    {
        _mutex->unlock();
        cout << "解锁成功" << endl;
    }
private:
    Mutex* _mutex;
};