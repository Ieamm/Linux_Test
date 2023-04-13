#include<iostream>
#include<pthread.h>
#include<unistd.h>

using namespace std;

void* backcall1(void* arg)
{
    string name = static_cast<char*>(arg);
    while(true)
    {
        sleep(1);
        cout << name  << " | pid:"<< getpid() << endl;
    }
}

void* backcall2(void* arg)
{
    string name = static_cast<char*>(arg);
    while(true)
    {
        sleep(1);
        cout << name  << " | pid:"<< getpid() << endl;    
    }
}

int main()
{
    pthread_t tid1,tid2;
    pthread_create(&tid1,nullptr,backcall1,(void*)"pthread1 running");
    pthread_create(&tid2,nullptr,backcall2,(void*)"pthread2 running");
    
    while(true)
    {
        sleep(1);
        cout << "main runing" << " | pid:"<< getpid() << endl;
    }

    pthread_join(tid1,nullptr);
    pthread_join(tid2,nullptr);

    return 0;
}

