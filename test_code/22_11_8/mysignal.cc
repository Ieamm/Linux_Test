#include <iostream>
#include <cstring>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

void childHandler(int sig)
{
    cout << "子进程暂停/恢复/结束,返回信号:" << sig << endl;
}

int main()
{
    //由于信号的三表(pending,block,handler)均是由进程PCB管理的.
    //因此,我们在父进程的控制流内将信号相关信息修改后,子进程会继承这些数据.
    signal(SIGCHLD, childHandler);
    int id = fork();
    if (id == 0) //子进程
    {
        while (true)
        {
            sleep(1);
            cout << "当前是子进程在运行,子进程ID为:" << getpid() << endl;
        }
        exit(0);
    }
    else //父进程
    {
        int status = 0;
        while (true)
        {
            sleep(1);
            pid_t cp = waitpid(id, &status, WNOHANG);
            if (cp > 0)
            {break;}
            else if (cp == 0) //未返回
            {
                cout << "当前是父进程在运行,父进程ID为:" << getpid() << endl;
                continue;
            }
            else
                {cout << "waitpid error" << endl;}
        }
    }
    cout << "child exit && main exit" << endl;
    sleep(3);
    return 0;
}

// killall
//  void handler(int sig)
//  {
//      cout << "当前信号为:" << sig << endl;
//      exit(1);
//  }

// int main()
// {
//     for(int i = 1; i <= 31; ++i)
//     {
//         signal(i,handler);
//     }

//     while(true)
//     {
//         sleep(1);
//     }
//     return 0;
// }

// volatile
//  int flag = 0;
//  void handler1(int sig)
//  {
//      flag = 1;
//      cout << "flag:0->1" << endl;
//  }
//  int main()
//  {
//      signal(2,handler1);
//      while(!flag)
//      {   sleep(1);
//          cout << "flag:" << flag << endl;
//      }
//      return 0;
//  }
//  void handler(int sig)
//  {
//      cout << "接收到一个信号[" << sig << "]" << endl;
//      //获取pending表
//      sigset_t pending;
//      while(true)
//      {
//          cout << "." << endl;
//          //获取pending表
//          sigpending(&pending);
//          for(int i = 1; i <= 31; ++i)
//          {
//              int isSet = sigismember(&pending,i);
//              if(isSet == 1)
//                  {cout << '1';}
//              else if(isSet == 0)
//                  {cout << '0';}
//              else
//              {
//                  cout << "\n" << "sigismember error" << strerror(errno) << endl;
//                  break;
//              }
//          }
//          cout << endl;

//         sleep(1);
//     }
// }

// int main()
// {
//     struct sigaction act,oact;
//     //指定处理行为
//     act.sa_handler = handler;
//     //暂时用不上.
//     act.sa_flags = 0;
//     //设置在2号信号的处理过程中的信号屏蔽字
//     sigemptyset(&act.sa_mask);
//     sigaddset(&act.sa_mask,3);
//     //添加信号处理行为
//     sigaction(2,&act,&oact);

//     while(true)
//     {
//         cout << "main runing" << endl;
//         sleep(1);
//     }

//     return 0;
// }