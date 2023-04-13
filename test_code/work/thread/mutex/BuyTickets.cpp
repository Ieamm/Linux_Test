#include<iostream>
#include<cstdio>
#include<pthread.h>
#include<unistd.h>
using namespace std;

//初始化锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//临界资源
int tickets = 1000;

void* Start_Rountine(void* arg)
{
    const char* msg = (char*)arg;
    while(1)
    {
        //从这里开始访问临界资源,加锁也应该从这里开始加
        pthread_mutex_lock(&mutex);
        if(tickets > 0)
        {
            --tickets;
            fprintf(stdout,"%s抢到了票,当前剩余票数:%d\n",msg,tickets);
            pthread_mutex_unlock(&mutex);  
        }
        else
        {
            fprintf(stdout,"%s发现票没了,不抢了,回家吃饭了.\n",msg);
            pthread_mutex_unlock(&mutex);
            break;
        }

        //注意,解锁不能简单的放在这个位置.
        //因为抢票机制的缘故,当票数<=0时,直接break,不会执行到解锁语句.
        //pthread_mutex_unlock();

        //因为我们希望充分的利用多线程的并发优势,因此,我们就不能让一个线程长时间的处于被调度状态.
        //当一个线程处于休眠状态时,就表示它被从CPU上剥离下来,也就代表有其他的任务被放在了CPU上运行.
        usleep(300);
    }
    return nullptr;
}

int main()
{
    pthread_t tid1,tid2,tid3;
    pthread_create(&tid1,nullptr,Start_Rountine,(void*)"1号抢票软件");
    pthread_create(&tid2,nullptr,Start_Rountine,(void*)"2号抢票软件");
    pthread_create(&tid3,nullptr,Start_Rountine,(void*)"3号抢票软件");

    while(1)
    {
        //因为这里也是对临界资源的访问,因此也需要加锁.
        pthread_mutex_lock(&mutex);
        if(tickets == 0)
        {
            fprintf(stdout,"都有票啦!上车回家!\n");
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
        
        fprintf(stdout,"没有票,等待抢票软件抢票\n");
        //这里的休眠同上,注意不要带着锁休眠.
        usleep(800);   
    }
    
    //回收子线程资源,防止"僵死".
    pthread_join(tid1,nullptr);
    pthread_join(tid2,nullptr);
    pthread_join(tid3,nullptr);
    
    return 0;
}