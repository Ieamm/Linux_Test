#include<iostream>
#include<unistd.h>
#include<pthread.h>
using namespace std;

pthread_rwlock_t rw_Mutex;

int border = 0;

//读者线程
void* reader(void *arg)
{
    while(true)
    {
        //加读锁
        pthread_rwlock_rdlock(&rw_Mutex);
        cout << "border:" << border << endl;
        //这里我们是在当前读者线程"持有锁"的状态下进行的休眠.
        sleep(1);
        pthread_rwlock_unlock(&rw_Mutex);
    }

    return nullptr;
}

//写者线程
void* writer(void *arg)
{
    sleep(1);
    while(true)
    {
        //加写锁
        pthread_rwlock_wrlock(&rw_Mutex);
        cout << "border:" << border << " ++border:" << ++border << endl;
        //每5秒一次递增操作,同样是持有锁状态下的休眠.
        sleep(1);
        pthread_rwlock_unlock(&rw_Mutex);
    }

    return nullptr;
}

int main()
{
    pthread_rwlock_init(&rw_Mutex,nullptr);
    pthread_t r1,r2,r3,r4,r5,r6,w1,w2;
    pthread_create(&r1,nullptr,reader,nullptr);
    pthread_create(&r2,nullptr,reader,nullptr);
    pthread_create(&r3,nullptr,reader,nullptr);
    pthread_create(&r4,nullptr,reader,nullptr);
    pthread_create(&r5,nullptr,reader,nullptr);
    pthread_create(&r6,nullptr,reader,nullptr);
    pthread_create(&w1,nullptr,writer,nullptr);
    pthread_create(&w2,nullptr,writer,nullptr);

    pthread_join(r1,nullptr);
    pthread_join(r2,nullptr);
    pthread_join(r3,nullptr);
    pthread_join(r4,nullptr);
    pthread_join(r5,nullptr);
    pthread_join(r6,nullptr);
    pthread_join(w1,nullptr);
    pthread_join(w2,nullptr);

    pthread_rwlock_destroy(&rw_Mutex);

    return 0;
}