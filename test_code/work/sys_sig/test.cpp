// #include<iostream>

// #include<unistd.h>
// #include<signal.h>
// using namespace std;

// void sigcb(int signal)
// {
//     cout << "信号" << signal << "的自定义递达处理行为" << endl;
// }

// int main()
// {
//     signal(2,sigcb);
//     sleep(10);
//     return 0;
// }

// #include<iostream>
// #include<cstring>
// #include<errno.h>
// #include<unistd.h>

// #include<signal.h>
// using namespace std;

// void sigcb(int signal)
// {
//     cout << "信号" << signal << "的自定义递达处理行为" << endl;
// }

// int main()
// {
//     struct sigaction act,oldact;//输入型参数(设置信号处理行为)和输出型参数(返回当前信号处理行为)
//     //以下两个结构体成员暂时用不到
//     act.sa_flags=0;
//     act.sa_restorer=nullptr;
//     //指定处理行为
//     act.sa_handler = sigcb;
//     //设置信号屏蔽字
//     sigemptyset(&act.sa_mask);

//     int sr = sigaction(2,&act,&oldact);
//     if(sr == -1)
//         cout << "sigaction error : " << strerror(errno) << endl;

//     sleep(20);
//     cout << "我被唤醒了!" << endl;
    
//     // sleep(1);
//     // cout << "我又睡下了!" << endl;
//     // sleep(5);
    
//     return 0;
// }

#include<iostream>
#include<signal.h>
#include<unistd.h>
using namespace std;

void current_sigpending_31(sigset_t *pendings)
{
    for(int i = 1; i <= 31; ++i)
    {
        int n = sigismember(pendings,i);
        if(n == 1)
            cout << '1';
        else if(n == 0)
            cout << '0';
        else
            cout << "error" << endl;
    }
    cout << endl;
}

void current_sigpending_64(sigset_t *pendings)
{
    for(int i = 34; i <= 64; ++i)
    {
        int n = sigismember(pendings,i);
        if(n == 1)
            cout << '1';
        else if(n == 0)
            cout << '0';
        else
            cout << "error" << endl;
    }
    cout << endl;
}

int main()
{
    sigset_t new_set,old_set;
    sigemptyset(&new_set);//初始化sigset_t对象
    sigaddset(&new_set,2);//屏蔽2号信号
    sigaddset(&new_set,40);//屏蔽40号信号
    sigprocmask(SIG_SETMASK,&new_set,&old_set);
    int sig_2 = 1;
    int sig_40 = 1;
    
    sigset_t pendings;
    sigisemptyset(&pendings);
    while(sig_2 != 6)
    {
        cout << "产生2号信号 :" << sig_2++ << endl;
        raise(2);
        sleep(1);
        sigemptyset(&pendings);
        if(sigpending(&pendings) == 0)
            current_sigpending_31(&pendings);
    }
    while(sig_40 != 6)
    {
        cout << "输入40号信号 : " << sig_40++ << endl;
        raise(40);
        sleep(1);
        sigemptyset(&pendings);
        if(sigpending(&pendings) == 0)
            current_sigpending_64(&pendings);
    }
    
    printf("解除所有信号屏蔽\n");
    sigprocmask(SIG_SETMASK, &old_set,nullptr);
    return 0;
}