//#include"Lock.hpp"
#pragma once
#include<iostream>
#include<queue>

#include<pthread.h>
using namespace std;

// 创建一个阻塞队列,模拟生产者消费者模型

template<class T>
class BlockQueue
{
public:
    BlockQueue(size_t cap=5):_cap(cap)
    {
        pthread_mutex_init(&_mutex,nullptr);
        pthread_cond_init(&_condc,nullptr);
        pthread_cond_init(&_condp,nullptr);
    }
    
    ~BlockQueue()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_condc);
        pthread_cond_destroy(&_condp);
    }
    
    //生产者
    void push(const T& in) // --传入一个只读参数.
    {
        //对于向队列中插入/生产,需要以下步骤:
        //1.生产者线程启动

        //2.加锁保护,并检查条件是否满足(即队列是否为满,若不为满则生产)
        LockQueue();
        //这里的循环检查是为了避免虚假唤醒的发生.
        //虚假唤醒:处于条件变量等待状态下的线程被唤醒,但程序员预期的条件并未满足,也意味着:唤醒!=条件满足.
        //需要唤醒后再判断一次条件
        //无论是人为代码错误的,还是CPU调度机制产生的.因此,为了避免虚假唤醒影响程序正确性,循环判断是最好的设计.
        while(isFull())
        {
            //2.1.若队列为满则阻塞等待
            //调用函数时将会执行原子的入等待队列+解锁操作.
            proBlockWait();
            //在返回时将会申请作为参数被传入的互斥量
        }
        //2.2.若队列不为满则继续执行
        
        //3.product Code
        Push_Core(in);

        //4.解锁--又因为当前队列里必然不为空,唤醒消费者线程.
        //4.1在临界区内唤醒消费者线程
        //先唤醒消费者线程,但因为消费者线程在wait返回时需要申请互斥量.
        //而我们并未解锁,因此消费者线程实际上还是处于阻塞等待状态--阻塞等待互斥锁.
        //但是并不影响程序正确性,因为当消费者线程以及其他线程休眠,生产者线程再度被调度时,继续向下执行自然会解锁.
        //此时,消费者线程就能正确的获得锁,并继续向下执行了---在没有其他优先级更高的互斥量申请的场景下.
        ////WakeupPro();
        
        unLockQueue();// -- 解锁
        
        //4.2在临界区外唤醒消费者线程
        //与上面相反,先解锁,解锁后可能生产者线程就被剥离,其他线程申请了锁在使用.
        //而我们的消费者线程需要等到被切换回生产者线程后才能被唤醒,同样的申请->等待锁.
        WakeupCon();
        //以上两种写法都有可能被优先级更高的互斥量申请线程提前抢占锁,以及其内部可能造成死锁的设计,造成饥饿问题.
        //但,这是其他线程的线程安全问题,不是我们生产-消费者的线程安全问题.
    }

    //消费者
    T pop()
    {
        //对于从队列中取出/消费,需要以下步骤:
        //1.消费者线程启动
        
        //2.加锁保护,并检查条件是否满足(即队列是否为空,若不为空则消费)
        LockQueue();
        //2.1.若队列为空则阻塞等待
        while(isEmpty())
        {
            conBlockWait();
        }
        //2.2.若队列不为空则继续执行
        
        //3.Consume Code
        T t = Pop_Core();
        
        //4.解锁--又因为当前队列里必然不为满,可以唤醒消费者线程消费.
        //4.1唤醒生产者线程
        WakeupPro();
        unLockQueue();
        return t;
    }

private:
    bool isFull()
    {
        return _bq.size() == _cap;
    }

    bool isEmpty()
    {
        return _bq.empty();
    }

    void LockQueue()
    {
        pthread_mutex_lock(&_mutex);
    }

    void unLockQueue()
    {
        pthread_mutex_unlock(&_mutex);
    }

    //生产线程等待条件变量满足
    void proBlockWait()
    {
        pthread_cond_wait(&_condp,&_mutex);
    }

    //消费线程等待条件变量满足
    void conBlockWait()
    {
        pthread_cond_wait(&_condc,&_mutex);
    }

    //唤醒生产线程
    void WakeupPro()
    {
        pthread_cond_signal(&_condp);
    }

    //唤醒消费线程
    void WakeupCon()
    {
        pthread_cond_signal(&_condc);
    }

    //生产核心代码
    void Push_Core(const T& in)
    {
        _bq.push(in);
    }

    //消费核心代码
    T Pop_Core()
    {
        //获取头
        T temp = _bq.front();
        //去头
        _bq.pop();
        return temp;
    }
private:
    queue<T> _bq;
    size_t _cap;
    pthread_mutex_t _mutex; //条件变量需要的互斥锁--用来保护临界资源
    pthread_cond_t _condp;  //消费者的就绪条件变量
    pthread_cond_t _condc;  //生产者的就绪条件变量
};