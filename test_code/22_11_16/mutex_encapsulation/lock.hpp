#pragma once
#include<iostream>
#include <pthread.h>

class Mutex
{
public:
    Mutex()
    {
        pthread_mutex_init(&_mutex, nullptr);//创建即初始化锁
        std::cout << "初始化锁...." << std::endl;
    }

    void lock()
    {
        pthread_mutex_lock(&_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&_mutex);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&_mutex);//析构即销毁锁
        std::cout << "解锁成功...." << std::endl;
    }

private:
    pthread_mutex_t _mutex;
};

class Lock_Guard
{
public:
    Lock_Guard(Mutex* p_mutex):_mutex(p_mutex)
    {
        _mutex->lock();//创建即加锁
        std::cout << "加锁成功...." << std::endl;
    }

    ~Lock_Guard()//自动调用"组件"Mutex的析构函数
    {
        _mutex->unlock();//析构即解锁
        std::cout << "解锁成功...." << std::endl;
    }
private:
    Mutex *_mutex;
};
