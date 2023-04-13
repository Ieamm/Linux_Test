#include<iostream>
#include<vector>
#include<functional>
#include<cstdio>

#include<pthread.h>
#include<unistd.h>

using namespace std;

///--认识条件变量接口--///

//定义全局条件变量--临界资源
pthread_cond_t cond;
//条件变量的配套设施---互斥锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//线程各自使用的方法集
vector<function<void()>> tfunc_set;
//线程参数结构体
typedef struct thread_parameter
{
    function<void()> _func;
    const char* _message;
}thread_parameter;

void show()
{cout << "hello world" << endl;}

void print()
{cout << "hello Linux" << endl;}

void init_funcset()
{
    //填充方法集
    tfunc_set.emplace_back(show);
    tfunc_set.emplace_back(print);
    tfunc_set.emplace_back([](){
        cout << "hello C Plus Plus" << endl;
    });
}

//为了避免编译器优化,直接从内存取数据
volatile bool quit = false;

void* Start_Rountine(void* arg)
{
    //避免子线程僵死. 
    //1.分离子线程,使其线程资源不需要显式回收.
    //pthread_detach(pthread_self());
    
    thread_parameter* ptp = (thread_parameter*)arg; 
    while(!quit)
    {
        //阻塞等待条件变量就绪(被pthread_cond_signal唤醒)
        pthread_cond_wait(&cond,&mutex);
        printf("%s被唤起,线程ID:%lld\n",ptp->_message,pthread_self());
        ptp->_func();
    }
    //因为pthread_cond_wait在返回后互斥量被上锁--原意是保护condition不被改变.
    //这也是为什么,若不在线程结束时解锁,其他线程使用同一个锁的线程就无法继续执行.
    //因为锁被当前线程把持.
    //而前面在循环时,线程hold的锁将因为执行pthread_cond_wait()函数,将线程放入等待条件的线程列表中被释放.
    //我们手动解锁,就是做了pthread_cond_wait()函数的工作.
    pthread_mutex_unlock(&mutex);
    return nullptr;
}
 

int main()
{
    //初始化条件变量
    pthread_cond_init(&cond,nullptr);
    //初始化方法集
    init_funcset();

    pthread_t tid1,tid2,tid3;
    int func_index = 0;
    thread_parameter tid1_tp,tid2_tp,tid3_tp;
    tid1_tp._func = tfunc_set[func_index++];
    tid2_tp._func = tfunc_set[func_index++];
    tid3_tp._func = tfunc_set[func_index++];
    tid1_tp._message = "子线程1";
    tid2_tp._message = "子线程2";
    tid3_tp._message = "子线程3";
    pthread_create(&tid1,nullptr,Start_Rountine,(void*)&tid1_tp);
    pthread_create(&tid2,nullptr,Start_Rountine,(void*)&tid2_tp);
    pthread_create(&tid3,nullptr,Start_Rountine,(void*)&tid3_tp);

    while(!quit)
    {
        //cout和cin先后使用,会直接刷新缓冲区.
        cout << "Enter(n/q/t)";
        char n;
        cin >> n;
        switch (n)
        {
        case 'n':
        {
            pthread_cond_signal(&cond);
            break;   
        }
        case 'q':
        {
            //更改quit临界资源,并所有线程.
            quit = true;
            //休眠一秒,避免由于调度问题产生的临界资源数据错误.
            sleep(1);
            //此时条件等待队列中的线程被唤醒,争取锁.-> 拿到锁的线程向下执行 -> 回到循环判断处 -> 
            //!quit为false退出循环 -> 解锁 -> 退出线程 -> 僵死待回收 -> 其他线程竞争锁.
            pthread_cond_broadcast(&cond);
            cout << "main thread quit" << endl;
            break; 
        }
        case 't':
        {
            pthread_cond_broadcast(&cond);
            break;
        }
        default:
            cout << "order error" << endl;
            break;
        }
    }

    //2.使用pthread_join回收子线程,避免子线程僵死
    pthread_join(tid1,nullptr);
    pthread_join(tid2,nullptr);
    pthread_join(tid3,nullptr);
    pthread_cond_destroy(&cond);

    return 0;
}

///--认识条件变量接口--///




// ///---单一锁场景下的死锁场景---///

// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// void* startRoutine(void* args)
// {
//     char* name = (char*)args;
//     while(true)
//     {
//         pthread_mutex_lock(&mutex);
//         cout << name << endl;
//         //本应该是解锁函数,但却调用错了接口,造成死锁.
//         //因此,即使是在单一锁的场景下,也会出现死锁情况.
//         pthread_mutex_lock(&mutex);
//         sleep(1);
//     }
    
// }
// struct timespec;
// int main()
// {
//     pthread_t tid1;
//     pthread_create(&tid1,nullptr,startRoutine,(void*)"thread1");
//     for(int i = 1; i <= 10; ++i)
//     {
//         cout << "main runing : " << i << endl;
//         sleep(1);
//     }
//     pthread_join(tid1,nullptr);
//     return 0;
// }

// ///---单一锁场景下的死锁场景---///