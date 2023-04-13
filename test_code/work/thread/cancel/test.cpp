#include<iostream>
#include<cstdio>
#include<pthread.h>

#include<unistd.h>
using namespace std;

void* Start_Routine(void* arg)
{
    //子执行流休眠,此时主执行流必定被调度.
    sleep(1);
    const char* msg = (char*)arg;
    for(int i = 1; i <= 5; ++i)
    {
        sleep(1);
        fprintf(stdout,"%s,seconds:%d\n",msg,i);
    }   
    return nullptr;
}

int main()
{
    sleep(2);
    pthread_t tid1;
    pthread_create(&tid1,nullptr,Start_Routine,(void*)"子线程");

    //主线程自我取消
    pthread_cancel(pthread_self());
    int time = 1;
    while(time != 10)
    {
        sleep(1);
        cout << "主线程工作中~~~ seconds:" << time++ << endl;
    }   
    return 0;
}