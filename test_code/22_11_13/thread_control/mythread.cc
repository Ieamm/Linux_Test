#include<iostream>
#include<cstdlib>
#include<string.h>
using namespace std;

#include<pthread.h>
#include<unistd.h>
#include<sys/syscall.h>

int global = 100;
void* startRoutine(void* args)
{
    pthread_detach(pthread_self());
    int cnt = 3;
    while(true)
    {
        sleep(1);
        cout << "LWP:" << syscall(SYS_gettid) << " | " <<(char*)args 
        << " | &global:" << &global  << " | Increment global:" <<++global<< endl;
        if(!(--cnt))
        {break;}
    }
}

int main()
{
    pthread_t tid1,tid2,tid3;
    pthread_create(&tid1,nullptr,startRoutine,(void*)"child thread 1");
    pthread_create(&tid2,nullptr,startRoutine,(void*)"child thread 2");
    pthread_create(&tid3,nullptr,startRoutine,(void*)"child thread 3");
    //三个线程创建完就直接被分离
    // pthread_detach(tid1);
    // pthread_detach(tid2);
    // pthread_detach(tid3);

    //对已经分离的线程进行等待会产生什么结果?
    int n1 = pthread_join(tid1,nullptr);
    cout << n1 << ":"<< strerror(n1) << endl;
    int n2 = pthread_join(tid2,nullptr);
    cout << n2 << ":"<< strerror(n2) << endl;
    int n3 = pthread_join(tid3,nullptr);
    cout << n3 << ":"<< strerror(n3) << endl;
    return 0;
}





// __thread int global = 100;

// void* startRoutine(void* args)
// {
//     while(true)
//     {
//         sleep(1);
//         cout << "LWP:" << syscall(SYS_gettid) << " | " <<(char*)args 
//         << " | &global:" << &global  << " | Increment global:" <<++global<< endl;
//     }
// }

// int main()
// {
//     pthread_t tid1,tid2,tid3;
//     pthread_create(&tid1,nullptr,startRoutine,(void*)"child thread 1");
//     pthread_create(&tid2,nullptr,startRoutine,(void*)"child thread 2");
//     pthread_create(&tid3,nullptr,startRoutine,(void*)"child thread 3");
    
//     pthread_join(tid1,nullptr);
//     pthread_join(tid2,nullptr);
//     pthread_join(tid3,nullptr);
//     return 0;
// }






// #include<iostream>
// #include<pthread.h>
// #include<unistd.h>
// using namespace std;

// int global = 101;

// static void PrintTid(const char* name, pthread_t tid,int * val)
// {
//     printf("thread name: %s | thread id  %lld | heap resource [ address : %p | value : %d ]\n",name,tid,val,*val);
// }

// void *startRoutine(void *args)
// {
//     //测试代码--64位下的类型大小
//     // cout << sizeof(void*) << endl;
//     // cout << sizeof(long) << endl;
    
//     //参数复原
//     const char* name = static_cast<const char*> (args);

//     //取消主线程
//     // pthread_t main_tid= reinterpret_cast<pthread_t>(args);
//     // cout << "main thread tid: " << main_tid << endl;
//     // int n = pthread_cancel(main_tid);
//     // cout << "取消主线程: " <<  n << endl;
//     // void * ret = nullptr;
//     // pthread_join(main_tid,&ret);
//     // cout << "child thread cancel main thread, ret : " << (long long)ret <<endl;

//     int cnt = 2;
//     int * pi = new int (15);
//     while(true)
//     {
//         //cout << "子线程倒计时:" << cnt << endl;
        
//         PrintTid(name,pthread_self(), pi);
//         sleep(1);
//         if(!(--cnt))
//         {
//             // cout << "修改global" << endl;
//             // global = 201;
//             break; 
//         }
//     }
//     PrintTid(name,pthread_self(),pi);
//     cout << "tid:" << pthread_self() <<"线程退出" << endl;

//     return (void*)pi;

//     //1.线程退出的方式1.直接return返回
//     //return (void*)1111;//这里是将一个值为1111的整形类型强转为了void*的指针
    
//     //2.线程退出的方式2.使用pthread_exit函数终止线程,并设置退出码
//     // pthread_exit((void*)56129);
// }

// int main()
// {
//     pthread_t tid;
//     //创建线程.
//     pthread_t main_tid = pthread_self();
//     cout << "main thread tid: " << main_tid << endl;
//     //pthread_create(&tid,nullptr,startRoutine,(void*)main_tid);
//     pthread_create(&tid,nullptr,startRoutine,(void*)"child thread");

//     // //创建完毕后主执行流继续向下
//     // int cnt = 3;
//     // while(1)
//     // {
//     //     cout << "主线程倒计时:" << cnt << endl;
//     //     sleep(1);
//     //     if(!(cnt--))
//     //         break;
//     // }
//     // cout << "创建子线程完毕" << endl;
//     // sleep(3);
    
//     // //3.线程退出的方式3.使用pthread_cancel函数取消线程,若一个线程是被取消的,则返回-1
//     // int n = pthread_cancel(tid);
//     // cout << "线程被取消,所返回值:" << n << endl;

//     //阻塞等待线程结束
//     void* ret = nullptr;//接收进程退出码的输出型参数
//     pthread_join(tid,&ret);
//     //因此这里应该将ret转换为,在子线程内部被转换成void*的指针之前的类型,即整形本身,而非整形指针.
//     cout << "ret : " << *((int*)ret) << endl;


//     while(true)
//     {
//         sleep(1);
//         PrintTid("main thread",main_tid,(int*)ret);
//     }
//     return 0;
// }
