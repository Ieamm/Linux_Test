#include<iostream>
#include<unistd.h>
#include<cstring>
#include<cstdio>
#include<sys/wait.h>
#include<sys/types.h>
#include<cstdlib>
#include<unordered_map>
#include<vector>
#include<ctime>
#include<cassert>
using namespace std;

typedef void (*functor)();

//方法集
vector<functor> functors;
//信息集
unordered_map<uint32_t,string> info;


void f1()
{
    cout << "这是一个处理日志的任务, 执行的进程 ID [" << getpid() << "]"
         << "执行时间是[" << time(nullptr) << "]" << endl;
}
void f2()
{
    cout << "这是一个备份数据任务, 执行的进程 ID [" << getpid() << "]"
         << "执行时间是[" << time(nullptr) << "]" << endl;
}
void f3()
{
    cout << "这是一个处理网络连接的任务, 执行的进程 ID [" << getpid() << "]"
         << "执行时间是[" << time(nullptr) << "]" << endl;
}

//将需要分发给子进程的任务放入进任务集合中.
//再用hashmap存储任务的编号与对应的任务信息.
void loadFunctor()
{
    info.insert({functors.size(), "处理日志的任务"});
    functors.push_back(f1);

    info.insert({functors.size(), "备份数据任务"});
    functors.push_back(f2);

    info.insert({functors.size(), "处理网络连接的任务"});
    functors.push_back(f3);
}

typedef pair<uint32_t, uint32_t> elem;//pair<pid,pipeFd--writer>;
int processNum = 5;//进程池内子进程数量
void work(int blockFd)
{
    cout << "进程:" << getpid() << "开始工作" << endl;
    //子进程工作代码
    while(true)
    {
        //阻塞等待父进程向子进程写入
        uint32_t taskNumber = 0;
        ssize_t ret = read(blockFd,&taskNumber,sizeof(uint32_t));
        //管道写端被关闭,读端也将所有管道文件缓冲区内的数据读取完毕,read返回0.
        if(ret == 0)
        {
            cout << "子进程任务结束, 进程销毁." << endl;
            break;
        }
        //断言没有出现字节流的错误读取.
        assert(ret == sizeof(uint32_t));
        (void)ret;
        //执行任务
        if(taskNumber < functors.size())
        {
            functors[taskNumber]();
        }
    }
    cout << "进程:" << getpid() << "结束工作" << endl;
}

void blanceSendTask(const vector<elem> &processFds)
{
    srand((long long)time(nullptr));
    int cnt = 0;
    while(cnt < 15)
    {
        sleep(1);
        //随机选择一个进程执行任务
        //较为均匀的将任务分发给子进程---负载均衡策略
        uint32_t pick = rand() % processFds.size();
        
        //随机选择任务
        uint32_t task = rand() % functors.size();

        //把一个任务交给一个指定的进程--->即,将控制信息写入控制进程与执行进程关联的管道文件内.
        write(processFds[pick].second, &task, sizeof(uint32_t));

        //打印对应的信息
        cout << "第[" << cnt << "]次" << "父进程指派任务->" << info[task] 
        << " 给进程: " << processFds[pick].first << " 编号: " << pick << endl;

        ++cnt;
    }
}

//通过父进程控制多个子进程的行为
int main()
{   
    loadFunctor();
    vector<elem> assignMap;//任务相关信息
    //循环创建子进程--->创建进程池
    for(int i = 0; i < processNum; ++i)
    {
        //创建管道
        int pipeFds[2] = {0};
        pipe(pipeFds);
        
        //子进程创建后继承了父进程的后序所有代码,包括这个循环内的代码.
        int id = fork();
        //也就是,每个子进程都执行了一次以下id==0内的代码.
        if(id == 0)
        {
            close(pipeFds[1]);
            //堵塞等待父进程向管道写入
            work(pipeFds[0]);
            close(pipeFds[0]);
            exit(0);
        }
        else if(id < 0)
        {
            cerr << "fork error" << endl;
            exit(1);
        }
        else//父进程
        {
            //关闭读端
            close(pipeFds[0]);
            elem e(id,pipeFds[1]);
            //将子进程的pid和与之关联的管道写端放入到容器内.
            assignMap.emplace_back(e);
        }
    }

    //分发任务
    blanceSendTask(assignMap);

    //处理资源
    for(int i = 0; i < processNum; ++i)
    {
        //阻塞等待每一个进程.
        //若其对应的管道文件的写端被关闭,读端将剩余在管道文件缓冲区内的数据读取完
        //跳出执行工作的代码,则代表该进程可以被关闭.
        if(waitpid(assignMap[i].first, nullptr, 0) > 0)
        {
            cout << "wait for: pid=" << assignMap[i].first << " wait success!"
                 << "number: " << i << "\n";
        }

        close(assignMap[i].second);
    }
    return 0;
}

// //父进程通过管道控制子进程行为
// int main()
// {
//     int pipefds[2] = {0};
//     if(pipe(pipefds))
//     {
//         cerr << "pipe error" << endl;
//     }  

//     loadFunctor();

//     int id = fork();
//     if(id == 0)//子进程
//     {
//         //先关掉无用的管道文件端口
//         //child->reader
//         close(pipefds[1]);
//         uint32_t taskNumber = 0;
//         while(true)//阻塞等待writer写入
//         {
//             memset(&taskNumber,0,sizeof(uint32_t));
//             //read返回读取的字节数--->对于管道文件,read在读取到结果之前会阻塞等待写入
//             ssize_t ret = read(pipefds[0], &taskNumber, sizeof(uint32_t));
//             if(ret == 0)//0代表文件结尾--->即没有写入端了.
//             {
//                 cout << "父进程写入结束,子进程读取完毕." << endl;
//                 break;
//             }
//             assert(ret == sizeof(uint32_t));
//             //assert断言仅在debug模式下有效
//             //在release模式下assert会被隐藏
//             //一旦断言消失,ret就成为了仅被初始化却没被使用的变量,会产生警告.
//             //这里对ret进行强转就是对ret的使用,消除了警告.
//             (void)ret;

//             if(taskNumber < functors.size())
//             {
//                 //调用对应方法集中对应的函数指针
//                 cout << "子进程接受任务,任务编号[" << taskNumber << "]. ";
//                 functors[taskNumber]();
//             }
//             else 
//             {
//                 cerr << "bug? taskNumber = " << taskNumber << endl;
//             }
            
//         }
//         close(pipefds[0]);
//         exit(1);
//     }
//     else if( id > 0) //父进程
//     {
//         srand((long long)time(nullptr));
//         //parent->writer
//         close(pipefds[0]);
//         int cnt = 0;
//         while(cnt < 10)//向管道进行写入
//         {
//             uint32_t sendNumber = rand() % functors.size();
//             sleep(1);
//             //向管道写入
//             write(pipefds[1], &sendNumber, sizeof(uint32_t));
//             //报告任务信息
//             cout << "第[" << cnt << "]次分发任务: " << "任务编号:[" << sendNumber 
//             << "] 任务内容: " << info[sendNumber] << endl;
//             ++cnt;
//         }
//         close(pipefds[1]);
        
//         //写入完毕,阻塞等待子进程读写完毕
//         if(waitpid(id,nullptr,0))//waitpid在等待成功后,返回等待到的进程pid
//         {
//             cout << "父进程等待子进程成功" << endl;
//         }
//     }
//     else 
//     {
//         cerr << "fork error" << endl;
//     }
//     return 0;
// }


// //演示pipe进程间通信的基本过程---匿名管道
// int main()
// {
//     int fdIds[2]={0};
//     int fd = pipe(fdIds);
//     if(fd < 0)
//     {
//         cerr << "pipe error" << endl;
//     }

//     int id = fork();
//     //子进程---这里我们让子进程从管道读取.
//     if(id == 0)
//     {
//         //关闭写入端
//         close(fdIds[1]);
//         char strBuffer[1024];
//         while(true)
//         {
//             memset(strBuffer,0,sizeof(strBuffer));
//             //每次都读取1023个字符,少读的一个位置留作'\0'
//             ssize_t len = read(fdIds[0],strBuffer,sizeof(strBuffer)-1);
//             if(len > 0)//读取成功,返回读取到的字节数
//             {
//                 //设长度为5,读取5个char也就是5个字节,最后一个字符在下标为4的位置
//                 //在下标5添加一个'\0',no problem
//                 strBuffer[len] = '\0';
//                 printf("子进程收到消息,内容是:%s\n",strBuffer);
//             }
//             else if(len == 0)//读取到文件结尾
//             {
//                 cout << "父进程写完,子进程返回" << endl;
//                 break;
//             }
//             else//读取错误
//             {
//                 //Nothing
//             }
//         }
//         exit(0);
//     }
//     else if (id < 0)
//     {
//         cerr << "fork error" <<endl;
//     }
//     //父进程---这里我们让父进程向管道写入
//     else
//     {
//         //管道的两端只有一个特性,是读非写,是写非读.
//         //因为pipe为进程开启了文件的读写两端,因此必须关闭一个不用的文件端.
//         close(fdIds[0]);
//         const char* msg = "hello pipe! cnt:";
//         int cnt = 0;
//         char strBuff[1024];
//         while(cnt < 5)
//         {
//             sleep(1);
//             sprintf(strBuff,"%s%d",msg,cnt);
//             write(fdIds[1],strBuff,strlen(strBuff));   
//             ++cnt;
//         }
//         //关闭写入端.此时文件的引用计数为0,读端在将管道内剩余数据读取后来到文件结尾.
//         close(fdIds[1]);
//         int status = 0;
//         //阻塞等待子进程
//         //回忆一下阻塞等待的本质,即:将当前进程挂起到等待队列,并将进程状态从R->S/T/D
//         pid_t ret = waitpid(id,&status,0);
//         if(ret > 0)
//         {
//             cout << "等待子进程成功." << endl;
//         } 
//     }
//     return 0;
// }
