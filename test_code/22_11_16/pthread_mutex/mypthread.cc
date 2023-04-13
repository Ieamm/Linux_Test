#include<iostream>
#include<pthread.h>
#include<unistd.h>
using namespace std;

//由于线程仅支持单参数,但又会出现需要多参数的线程.
//定义一个传给线程的数据结构体.
struct Pthread_Data
{
    //发送个线程的锁
    pthread_mutex_t* _pmutex;
    string _name;    
};

int tickets = 10000;

void* PayTicket(void* args)
{
    Pthread_Data* arg = (Pthread_Data*)args;
    
    //加锁
    pthread_mutex_lock(arg->_pmutex);
    while(true)
    {
        if(tickets > 0)
        {
            --tickets;
            cout << arg->_name << " 抢到票了,剩余票数: "<< tickets << endl;
            pthread_mutex_unlock(arg->_pmutex);//对临界资源的访问结束,解锁,出让锁的所有权.
        }
        
        else
        {
            //没有访问到临界资源,也需要解锁.
            //若不解锁,一旦当前线程被调度走,或直接break,就会导致锁被当前线程带走.
            //其他阻塞等待锁的线程无法申请到锁,只能继续等待.
            pthread_mutex_unlock(arg->_pmutex);
            break;
        }
        usleep(42);//进入S/T状态,被挂起到等待队列,出让调度权,也就是使线程频繁切换
    }
    cout << arg->_name << " 放弃抢票,票已经被抢完了,当前票数: " << tickets << endl;
}

int main()
{
    //初始化线程参数
    pthread_t tid1,tid2,tid3;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, nullptr);//初始化锁
    Pthread_Data* arg1 = new Pthread_Data();
    arg1->_pmutex = &mutex;
    arg1->_name = "thread1";
    Pthread_Data* arg2 = new Pthread_Data();
    arg2->_pmutex = &mutex;
    arg2->_name = "thread2";
    Pthread_Data* arg3 = new Pthread_Data();
    arg3->_pmutex = &mutex;
    arg3->_name = "thread3";

    //创建线程
    pthread_create(&tid1,nullptr,PayTicket,(void*)arg1);
    pthread_create(&tid2,nullptr,PayTicket,(void*)arg2);
    pthread_create(&tid3,nullptr,PayTicket,(void*)arg3);

    //等待线程结束
    pthread_join(tid1,nullptr);
    pthread_join(tid2,nullptr);
    pthread_join(tid3,nullptr);

    pthread_mutex_destroy(&mutex);
    return 0;
}
