#pragma once
#include"Lock.hpp"

#include<iostream>
#include<queue>
#include<ctime>
#include<cstdlib>
#include<assert.h>

#include<unistd.h>
#include<pthread.h>

using namespace std;

const int Def_threadNumber = 5;

template<class T>
class threadPool
{
public:
    threadPool<T>& operator=(const threadPool<T>&) = delete;
    threadPool(const threadPool<T>&) = delete;
public:
    //由于线程池的特性,我们可以将其编写为单例模式(singleton).
    static threadPool<T>* getInstance()
    {
        static Mutex mutex;
        if(instance == nullptr)
        {
            LockGuard lockgurad(&mutex);//进入代码块加锁,退出代码块解锁.
            if(instance == nullptr)
            {
                instance = new threadPool<T>();
            }
        }
        return instance;
    }

    ~threadPool()
    {
        pthread_cond_destroy(&_cond);
        pthread_mutex_destroy(&_mutex);
    }
    
    //由于类内成员均有一个默认隐藏的类对象指针--this指针
    //因此,线程函数不能直接写在类内--因为不符合pthread_create的参数要求.
    //为此,我们可以将其写为static函数.
    static void* threadRountine (void *arg)
    {
        cout << "threadRoutine" << endl;
        pthread_detach(pthread_self());
        threadPool<T>* ptp = static_cast<threadPool<T>*>(arg);
        
        //每个线程都对任务队列进行轮巡检测,以将任务分派
        while(1)
        {
            ptp->lockQueue();
            //防止虚假唤醒
            while(ptp->emptyTask())
            {
                ptp->waitForTask();
            }
            //执行到这里,证明队列内有任务被压入,该线程可以执行任务了.
            
            //即使是static函数,也是属于类内的.因此可以访问private成员
            T t = ptp->pop();
            ptp->unlockQueue();

            //执行任务.
            t();
        }

        return nullptr;
    }

    void start()
    {
        //不允许重复启动线程池
        assert(!_isStart);
        for(int i = 1; i <= _threadNumber; ++i)
        {
            pthread_t tid;
            pthread_create(&tid,nullptr,threadRountine,this);
        }

        _isStart = true;//更改当前线程池的状态,即已经启动.
    }

    //压入任务
    void push(const T& in)
    {
        //STL容器是线程不安全的,而push函数有被多线程访问的场景,因此需要加锁保护
        lockQueue();
        _taskQueue.push(in);
        choiceThreadForHandler();
        unlockQueue();
    }

private:
    threadPool(size_t threadNumber = Def_threadNumber):
    _isStart(false),
    _threadNumber(threadNumber)
    {
        pthread_mutex_init(&_mutex,nullptr);
        pthread_cond_init(&_cond,nullptr);
    }
    
    void lockQueue()
    { pthread_mutex_lock(&_mutex); }
    void unlockQueue()
    { pthread_mutex_unlock(&_mutex); }
    bool emptyTask()
    { return _taskQueue.empty(); }
    void waitForTask()
    { pthread_cond_wait(&_cond,&_mutex); }
    void choiceThreadForHandler()
    { pthread_cond_signal(&_cond); }
    
    T pop()
    {
        T t = _taskQueue.front();
        _taskQueue.pop();
        return t;
    }

private:
    bool _isStart;//判断线程池是否启动
    size_t _threadNumber;
    queue<T> _taskQueue;
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;

    static threadPool<T> *instance;
};

template<class T>
threadPool<T>* threadPool<T>:: instance = nullptr;