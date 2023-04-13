#include<iostream>

#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
using namespace std;


//2."不使用"我们的SIGCHLD信号
int main()
{
    signal(SIGCHLD,SIG_IGN);
    for(int i = 0; i < 10; ++i)
    {
        int id = fork();
        int cnt = 10;
        if (id == 0) //子进程
        {
            while (cnt--) // 10S后结束当前子进程
            {
                cout << "child runing | child pid:" << getpid() 
                << " | child time remaining:" << cnt << endl;
                sleep(1);
            }
            cout << "child process [" << getpid() << "] "
                 << "exit --- into zombie" << endl;
            exit(0);
        }
    }
    while(true)
    {
        sleep(1);
        cout << "main runing" << endl;
    }
    return 0;
}



//1.使用我们的SIGCHLD信号
// void FreeChildProcess (int signo)
// {
//     //当我们来到FreeChildProcess这个信号递达处理内部时,我们的程序就又新创建了一个控制流
//     while(true)
//     {
//         // //等待任意子进程运行完毕---阻塞等待
//         // int cp = waitpid(-1,nullptr,0);
//         // if(cp > 0)
//         // {    cout << "等待成功,子进程id为:"<< cp << endl;        }
//         // else
//         // {
//         //     cout << "等待子进程失败---没有任何正在运行的子进程" << endl;
//         //     break;
//         // }

//         等待任意子进程运行完毕---非阻塞等待.
//         成功返回child pid,没等到返回0,没有正在运行的子进程返回-1
//         int cp = waitpid(-1,nullptr,WNOHANG);
//         if(cp > 0)
//         {   cout << "等待成功,子进程id为:"<< cp << endl;    }
//         else if(cp = 0)
//         { cout << "还没等到,继续轮巡检测." << endl; }
//         else
//         {
//             cout << "等待子进程失败---没有任何正在运行的子进程" << endl;
//             break;
//         }
//     }
// }

// int main()
// {
//     signal(SIGCHLD,FreeChildProcess);
//     for(int i = 1; i <= 5; ++i)
//     {
//         int id = fork();
//         int cnt;
//         if(i < 4)
//         {cnt = 3;}
//         else
//         {cnt = 10;}
//         if(id == 0)//子进程
//         {    
//             while(cnt--)//10S后结束当前子进程
//             {
//                 cout << "child runing | child pid:" << getpid() << 
//                 " | child time remaining:" << cnt << endl;
//                 sleep(1);
//             }
//             cout << "child process [" << getpid() << "] "  
//             << "exit --- into zombie" << endl;
//             exit(0);
//         }
//         //子进程在创建之初,继承了父进程的上下文数据和PCB结构,mm_struct,页表
//         //再向下执行,运行上段代码,而父进程继续运行该循环,待循环结束跳出继续向下执行
//     }
    
//     while(true)
//     {
//         cout << "main runing" << endl;  
//         sleep(1);
//     }
    
//     return 0;
// }