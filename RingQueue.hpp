#include<iostream>
#include<vector>
#include<cstdlib>
#include<ctime>
#include<string>

#include<unistd.h>
#include<sys/types.h>
#include<semaphore.h>
#include<pthread.h>
using namespace std;

//使用互斥量模拟循环阻塞队列

const int DefaultCap = 5;

template<class T>
class RingBlockQueue
{
public:
    RingBlockQueue(size_t cap = DefaultCap):_cap(cap),_rbq(cap),_pindex(0),_cindex(0)
    {
        //循环队列创建时,其中没有数据,可用空间计数为队列大小.
        sem_init(&_dataSem,0,_cap);
        //循环队列创建时,其中没有数据,因此数据计数为0.
        sem_init(&_roomSem,0,_cap);
        
        pthread_mutex_init(&_proMutex,nullptr);
        pthread_mutex_init(&_conMutex,nullptr);
    }
    ~RingBlockQueue()
    {
        sem_destroy(&_dataSem);
        sem_destroy(&_roomSem);
        pthread_mutex_destroy(&_proMutex);
        pthread_mutex_destroy(&_conMutex);
    }
    
    //生产者
    void push(const T&in)
    {
        //申请空间互斥量,若空间互斥量不为0,则代表此时有空间插入数据,空间计数--.
        //若空间互斥量为0,则阻塞等待互斥量申请成功.
        sem_wait(&_roomSem);
        
        //因为申请空间计数器,本质上是为了访问_rbq这一资源.
        //又因为空间计数器可能为复数(非二元互斥量).与_pindex一样,_rbq也作为临界资源被多个执行流(多次push操作)共享.
        //由于STL容器的线程不安全性,_rbq在多执行流访问场景下需要加锁保证程序的线程安全.
        //也就是:当生产者线程希望访问到_rbq时,需要先申请到_proSem(空间计数器).申请的到则表示资源中有当前线程的一份.
        //但!申请到_proSem后,还需要再等待锁才能访问到:
        //这是为了线程安全做考虑,防止多个拿到了计数器的线程同时访问临界资源,造成的数据二义性.
        pthread_mutex_lock(&_proMutex);
        
        _rbq[_pindex] = in;     //生产任务
        _pindex++;              //写入位置后移
        _pindex %= _rbq.size(); //更新下标,使其保证环状队列特性
        
        //操作临界资源结束 -- 这里的解锁操作无论放在末尾还是放在这里都是可以的. 
        //因为生产者与消费者使用不同的锁.
        //且计数器的申请是阻塞等待的,解锁后直接被切走也不会造成错误的申请行为--因为此时还没有可申请的计数器.
        pthread_mutex_unlock(&_proMutex);
        
        //当数据插入成功时,数据计数++;
        sem_post(&_dataSem);
    }

    //消费者
    T pop()
    {
        //申请计数器
        sem_wait(&_dataSem);
        
        //申请成功计数器--,并等待锁
        pthread_mutex_lock(&_conMutex);

        T t = _rbq[_cindex];    //消费任务
        _cindex++;              //消费完毕,位置向后移
        _cindex %= _rbq.size(); //更新下标,使其保证环状队列特性

        //操作临界资源结束,解锁.
        pthread_mutex_unlock(&_conMutex);

        //数据减少,则空间变多.空间计数器++
        sem_post(&_roomSem);

        return t;
    }

private:
    size_t _cap;
    vector<T> _rbq;//实现循环队列的适配器
    sem_t _dataSem;//数据计数器
    sem_t _roomSem;//空间计数器
    uint32_t _pindex;//当前生产者写入的位置,在多线程场景下,属于临界资源.
    uint32_t _cindex;//当前消费者读取的位置,在多线程场景下,属于临界资源.
    
    pthread_mutex_t _proMutex;//生产者锁.
    pthread_mutex_t _conMutex;//消费者锁.
};

