#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "lock.hpp"
using namespace std;

pthread_mutex_t mutexB;
pthread_mutex_t mutexA;

//死锁问题
void* StartRoutine1(void* args)
{
    pthread_mutex_lock(&mutexA);
    pthread_mutex_lock(&mutexB);
    
    cout << "thread 1..." << endl;
    
    pthread_mutex_unlock(&mutexA);
    pthread_mutex_unlock(&mutexB); 
}

void* StartRoutine2(void* args)
{
    pthread_mutex_lock(&mutexB);
    pthread_mutex_lock(&mutexA);
    
    cout << "thread 2..." << endl;
    
    pthread_mutex_unlock(&mutexA);
    pthread_mutex_unlock(&mutexB); 
}

//定义一个全局的锁.
Mutex mutex;

int tickets = 10000;

void *PayTicket(void *args)
{
    char *arg = (char *)args;
    while (true)
    {
        usleep(42); //进入S/T状态,被挂起到等待队列,出让调度权,也就是使线程频繁切换
        //加锁
        Lock_Guard lock(&mutex);
        if (tickets > 0)
        {
            --tickets;
            cout << arg << " 抢到票了,剩余票数: " << tickets << endl;
        }
        else
        {
            cout << arg << " 放弃抢票,票已经被抢完了,当前票数: " << tickets << endl;
            break;
        }
        //由于lock_guard类的析构函数为封装后的解锁函数,lock对象又是一个建立在栈区的局部对象.
        //因此,当循环结束,代码作用域随之结束,析构函数被调用时就完成了解锁.
        //这也是一种RAII机制的体现.
    }
}

int main()
{
    //初始化线程参数
    pthread_t tid1, tid2, tid3;

    //创建线程
    pthread_create(&tid1, nullptr, PayTicket, (void *)"thread1");
    pthread_create(&tid2, nullptr, PayTicket, (void *)"thread2");
    pthread_create(&tid3, nullptr, PayTicket, (void *)"thread3");

    //等待线程结束
    pthread_join(tid1, nullptr);
    pthread_join(tid2, nullptr);
    pthread_join(tid3, nullptr);
    return 0;
}
