#include"daemonize.hpp"
#include"log.hpp"
#include"threadPool.hpp"
#include"util.hpp"   
#include"task.hpp"

#include<iostream>

#include<pthread.h>
#include<sys/wait.h>

#define SOCKET_ERR  1
#define BIND_ERR    2  
#define LISTEN_ERR  3
#define IP_INVALID  4

//声明 class tcpServer是一个类型

class tcpServer;

//执行命令函数
void execCommand(int sock, const string& clientIp, const uint16_t clientPort)
{
    assert(sock >= 0);
    assert(!clientIp.empty());
    assert(clientPort >= 1024);//0~1023都是系统预留接口.

    char command[BUFFER_SIZE];
    while(true)
    {
        //read和write函数本身就是流式传输/接收,因此对于TCP传输协议的两端而言,可以使用read和write进行通信.
        ssize_t s = read(sock, command, BUFFER_SIZE - 1);//因为我们接收到的都是字符串,需要最少留下一个空间加'\0'
        if(s > 0)
        {
            command[s] = '\0';
            logMessage(DEBUG,"[%s:%d] exec [%s]\n",clientIp.c_str(),clientPort,command);
            //考虑安全问题,禁止一些指令被执行
            const string safe = command;
            //若是找到这些禁止指令,则强制退出本次执行.
            if(string::npos != safe.find("rm") || string::npos != safe.find("unlink"))
            {
                logMessage(WARNING,"indisable instruction : %s\n",command);
                break;
            }

            //创建子进程执行command命令,将命令的执行结果通过匿名管道写到返回值FILE*指向的文件中.
            FILE *fp = popen(command,"r");
            if(fp == nullptr)
            {
                logMessage(WARNING,"exec %s faild beacuse: %s\n", command, strerror(errno));
                break;
            }

            char line[1024];
            //每次将fp指向的文件中的数据向缓冲区line中读取一行
            while(fgets(line, sizeof(line)-1, fp) != nullptr)
            {
                //将读取到的一行写入到对端.
                write(sock,line,strlen(line));
            }
            //将会阻塞到popen函数返回,用来关闭popen函数返回的文件流指针.
            pclose(fp);
            logMessage(DEBUG,"[%s:%d] exec:[%s]...done\n", clientIp.c_str(), clientPort, command);
        }
        else if(s == 0)//写端关闭.
        {
            logMessage(DEBUG,"client quit -- %s[%d]\n",clientIp.c_str(),clientPort);
            break;
        }
        else
        {
            logMessage(WARNING,"read error %s\n",strerror(errno));
            break;
        }
    }

}

struct ThreadData
{
public:
    int serviceSock;
    uint16_t clientPort;
    std::string clientIp;
};


class tcpServer
{
public:
    tcpServer(int port, const string ip):_port(port), _ip(ip), _listenSock(-1), _tp(nullptr), _quit(false)
    {}
    
    ~tcpServer()
    {
        if(_listenSock > -1)
            close(_listenSock);
    }

public:
    void init()
    {
        //1.创建套接字--TCP是流式传输的传输协议
        _listenSock = socket(AF_INET,SOCK_STREAM,0);
        if(_listenSock == -1)
        {
            logMessage(FATAL,"socket error %s %d\n",strerror(errno),_listenSock);
            exit(SOCKET_ERR);
        }
        logMessage(DEBUG,"socket success:%d\n",_listenSock);

        //2.将信息绑定到内核区域,等待被跨网络访问.
        //2.1初始化服务端信息结构体
        struct sockaddr_in local;
        socklen_t len = sizeof(local);
        memset(&local,'\0',len);
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);
        // //ip的网络序列化--写法1.方便但不完善,会因为错误的ip格式影响程序结果.
        // _ip.empty() ? local.sin_addr.s_addr = INADDR_ANY : local.sin_addr.s_addr = inet_addr(_ip.c_str());
        // ip的网络序列化--写法2.不方便但完善,对ip格式的正确性进行检验
        if(_ip.empty())
        {local.sin_addr.s_addr = INADDR_ANY; }
        else
        {
            if(inet_aton(_ip.c_str(),&local.sin_addr) == 0)
            {
                logMessage(FATAL,"invalid ip address %s\n",_ip.c_str());
                exit(IP_INVALID);
            }
        }
        //2.2bind
        if(bind(_listenSock,(const sockaddr*)&local,len) == -1)
        {
            logMessage(FATAL,"bind error %s %d\n",strerror(errno),_listenSock);
            exit(BIND_ERR);
        }
        logMessage(DEBUG,"bind success:%d\n", _listenSock);

        //3.创建监听套接字
        if(listen(_listenSock, Def_threadNumber) == -1)
        {
            logMessage(FATAL,"listen error : %s\n", strerror(errno));
            exit(LISTEN_ERR);
        }
        logMessage(DEBUG,"listen success:%d\n", _listenSock);

        //4.加载线程池
        _tp = threadPool<task>::getInstance();
    }

    void loop()
    {
        // //1.1启动线程池
        _tp->start();
        logMessage(DEBUG,"threadPool start, thread number:%d\n",Def_threadNumber);

        //退出标记未置位便一直循环.
        while(!_quit)
        {
            //2.从监听套接字的未决连接队列中获取连接--accept
            //2.1 用来接收创建连接的对端的信息
            struct sockaddr_in peer; //输出型参数
            socklen_t len = sizeof(peer); //输入输出型参数--分别代表传进去的结构体大小,以及接收的结构体大小
            memset(&peer, 0, len);
            //2.2创建服务套接字
            //申请连接的远端不止一个,而监听套接字也不会用来提供对端服务,仅有"监听"这一个功能.
            //因此,对连接到服务端的远端来说,服务端都需要为其单独的有一个socket,只服务于单一远端--serviceSock
            int serviceSock = accept(_listenSock, (sockaddr*)&peer, &len);
            //由于accept是阻塞等待,所以可能会出现发送退出信号置位退出标志以终止进程时,由于accept的阻塞导致\
            额外又进行了一次服务.因此在accept后脱离阻塞后再检测一次退出标记.
            if(_quit)   {break;}
            if(serviceSock == -1)
            {
                //我们的服务器面向不止一个远端,因此单一远端的连接失败不应该终止服务.
                logMessage(WARNING,"accept error : %s %d\n", strerror(errno), serviceSock);
                continue;
            }
            //2.3获取客户端信息
            const string peer_ip = inet_ntoa(peer.sin_addr);
            uint16_t peer_port = ntohs(peer.sin_port);

            //3.提供服务 -- 远端在主机上输入指令,服务端将指令在服务端上的运行结果push到远端
            //3.0 -- 单进程下直接进行服务,只有该服务完成后才能继续循环,继续accept.
            // execCommand(serviceSock, peer_ip, peer_port);
            // close(serviceSock);

            //5.1-v1 -- 多进程版本 -- 本质上与上面并无太大区别,也是串行化执行.
            // pid_t id = fork();
            // //判断创建子进程是否失败.
            // assert(id != -1);
            // if(id == 0)//子进程
            // {
            //     //我们在主进程中已经用_listenSock获取到了连接,因此在子进程中不再需要使用.
            //     //而文件描述符本身就是一种资源,放着一个不会再被使用的资源而不释放,就可以看作是一种"资源泄露"
            //     close(_listenSock);
            //     execCommand(serviceSock,peer_ip,peer_port);//提供服务
            //     exit(0);//进入僵尸
            // }
            // //父进程
            // close(serviceSock);//一定要做!理由同上面关闭_listenSock
            // waitpid(id,nullptr,0);//阻塞等待回收子进程

            // //5.1-v1.1 -- 多进程并发运行版本
            // pid_t id = fork();//爷爷线程创建父亲线程
            // if(id == 0)//父亲线程进入
            // {
            //     //父亲进程复制了爷爷进程的结构和数据,也继承了文件描述符.
            //     //文件描述符作为资源,若不需要再使用就应该关闭.
            //     close(_listenSock);
            //     //父亲进程再进行一次fork,创建儿子进程.
            //     if(fork() > 0) exit(1);//父亲进程直接退出,陷入僵尸态
            //     //儿子进程此时就成为了孤儿进程,被操作系统领养,回收问题交由操作系统来处理.
            //     execCommand(serviceSock,peer_ip,peer_port);//儿子进程提供服务
            //     exit(1);//服务结束后儿子进程退出.
            // }
            // close(serviceSock);//因为血缘进程间的写时拷贝的缘故,爷爷进程直接关闭文件描述符不会影响到父亲进程.
            // pid_t ret = waitpid(id,nullptr,0);//因为父亲进程的直接退出,可以立马得到退出码并释放僵尸状态.
            // assert(ret > 0);
            // (void)ret;//assert可能在release模式下被优化掉,ret就将因为"创建但未使用的变量"导致报警.


            //5.2 v2 -- 多线程版本
            //在多线程环境下,就不需要再关闭文件描述符了.
            //多线程处在同一个进程下,共享进程内的资源,文件描述符也不例外.
            // pthread_t tid;
            // ThreadData *td = new ThreadData();
            // td->clientIp = peer_ip;
            // td->clientPort = peer_port;
            // td->serviceSock = serviceSock;
            // pthread_create(&tid,nullptr,threadRoutine,(void*)td);

            //5.3 v3.1 -- 线程池任务版本
            task t(serviceSock,peer_ip,peer_port,execCommand);
            _tp->push(t);//压入任务,开始执行.
        }
    }

    void quitServer()
    {
        _quit = true;
    }

private:
    uint16_t _port;     //服务器自身端口
    std::string _ip;    //服务器自身IP
    int _listenSock;    //监听套接字
    threadPool<task>* _tp;   //单例线程池
    bool _quit;         //退出标志
};

tcpServer *svrp = nullptr;        
void quitHandler(int signal)
{
    //确认信号为3号信号,且当前服务器被实例化启动中.
    if(signal == 3 && svrp != nullptr)
        svrp->quitServer();
    
    logMessage(DEBUG,"Server quit save!\n");
}

void useManual()
{
    std::cout << "command port [ip]" << std::endl;
}

int main(int argc, char* args[])
{
    if(argc != 2 && argc != 3)
    {
        useManual();
        exit(1);
    }

    //精灵进程化
    //daemonize();

    uint16_t port = atoi(args[1]);
    string ip = args[2] == nullptr ? "" : args[2];
    svrp = new tcpServer(port,ip);

    signal(3,quitHandler);
    
    svrp->init();
    svrp->loop();

    return 0;
}
