#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <cstdlib>

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

void handler(int sig)
{
    cout << "当前进程收到了一个信号,信号编号为:" << sig << endl;
}

static void showPending(sigset_t *pendings)
{
    for (int i = 1; i <= 31; ++i)
    {
        if (sigismember(pendings, i) == 1)
        {
            cout << '1';
        }
        else
        {
            cout << "0";
        }
    }
    cout << endl;
}

int main()
{
    // 3. 设置屏蔽信号
    // sigfillset();
    sigset_t blocks;
    sigset_t outset;
    sigemptyset(&blocks);
    sigemptyset(&outset);
    for (int i = 1; i <= 31; ++i)
    {
        // 3.1 屏蔽信号
        sigaddset(&blocks, i);
        //设置信号的自定义行为
        signal(i,handler);
    }
    // 3.2 设置用户级的信号屏蔽字到内核中，让当前进程屏蔽到2号信号
    sigprocmask(SIG_SETMASK, &blocks, &outset);

    // 1. 不断的获取当前进程的pending信号集
    sigset_t pendings;
    sigemptyset(&pendings);

    int cnt = 0;
    while (true)
    {
        sleep(1);
        // 1.1清空信号集
        sigemptyset(&pendings);
        // 1.2获取当前进程的未决信号集
        if (sigpending(&pendings) == 0)
        {
            showPending(&pendings);
        }
        else
        {
            cerr << "sigpending error : " << strerror(errno) << endl;
        }
        ++cnt;

        if(cnt == 20)
        {
            cout << "解除对所有信号的屏蔽..." << endl;
            sigset_t Empty;
            sigemptyset(&Empty);
            if(sigprocmask(SIG_SETMASK,&Empty,nullptr) == -1)
            {
                cerr << "sigprocmask set Empty Block error : " 
                << strerror(errno) << endl;
            }
            
            break;
        }
    }

    /// Core Dump///
    // pid_t id = fork();
    // if (id == 0) //子进程
    // {
    //     //野指针
    //     int *pi = NULL;
    //     *pi = 15;
    //     exit(1);
    // }
    // else //父进程
    // {
    //     int status = 0;
    //     waitpid(id, &status, 0);
    //     // 1.求高八位.2.求低七位.3.求第八位.
    //     cout << "exitcode:" << (status >> 8) << " signo:" << (status & 0x7f)
    //          << " core dump" << ((status >> 7) & 0x1) << endl;
    // }

    // for(int i = 1; i <= 31; ++i)
    // {
    //     signal(i,handler);
    // }

    // //异常捕获除零错误---没整明白
    // try{
    //     int i = 100;
    //     i /= 0;
    // }
    // catch(...){
    //     cout << "程序发生除零错误,需要终止" << endl;
    //     abort();
    // }

    // //越界访问
    // int arr_I[100];
    // arr_I[10000] = 155;

    // //野指针
    // int* pi = NULL;
    // *pi = 15;

    return 0;
}
