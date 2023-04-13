#include "Sock.hpp"

#include <string>

#include <poll.h>
#include <unistd.h>
#include <iostream>

using namespace std;

#define DFL -1

const int NUM = (sizeof(fd_set) * 8);

struct pollfd fdArray[NUM];
const int gnum = sizeof(fdArray) / sizeof(fdArray[0]);

void ShowArray()
{
    for(int i = 0; i < gnum; ++i){
        if(fdArray[i].fd != DFL)
            cout << "第" << i << "位的fd为:" << fdArray[i].fd << endl;
    }
}

void useage()
{ 
    cout << "./Server Port [ip]"
         << "example:"
         << "./Server 8080 127.0.0.1" << endl;
}

void work()
{
    cout << "处理其他工作中~~~" << endl;
}

void Excepter(struct pollfd& conn);

int Reader(struct pollfd &conn)
{
    //TODO
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    //BUG --- TCP是流式传输
    int n = read(conn.fd, buffer, sizeof(buffer));
    if(n > 0)
    {
        buffer[n] = '\0';
        string out = buffer;
        cout << out << endl;
    }
    else if(n == 0)
    {
        cout << "客户端断开连接" << endl;
        Excepter(conn);
    }
    else
    {
        cout << "读取错误, 返回错误码:" << errno << ": " << strerror(errno) << endl;
        Excepter(conn);
    }
}

int Writer(struct pollfd &conn)
{

}

void Excepter(struct pollfd &conn)
{
    close(conn.fd);
    conn.fd = DFL;
}

// 4.事件就绪后进行处理
void ReadHandler(int listenSock)
{
    for(int i = 0; i < gnum; ++i)
    {
        if(fdArray[i].fd == DFL)  { continue; }
        
        //该下标的文件描述符的读事件准备就绪
        if(fdArray[i].revents & POLLIN)
        {
            //监听socket在获取新连接时,底层是要经过三次握手的，握手由Client端调用connect发起,由双方的传输层协议进行确认和连接建立。
            // 这里需要明确的是：网络协议栈本身存在于操作系统内部，是由操作系统内核通过软件的方式进行管理的。也就是说，TCP连接的实际建立，都是在内核中完成的。
            // accept的工作只是将已经在内核中被建立好, 存放在未决连接队列的TCP连接从内核空间拿到用户空间---本质就是数据拷贝的一种.
            //因此,对于底层有建立好的连接这一情况,我们可以称之为:读事件就绪!
            if(i == 0 && fdArray[i].fd == listenSock)
            {
                string connectIp;
                uint16_t connectPort;
                int connect = Sock::Accept(listenSock, &connectIp, &connectPort);
                if(connect < 0)
                {   cout << "Accept Error" << endl; }

                //将新连接纳入到管理数据结构中.
                //因为select多路转接策略对文件描述符的监控有着数组数量的限制,且文件描述符之间不存在次序问题,因此这里采用遍历检测的方法,节省空间.
                int i = 0;
                for(; i < gnum; ++i)
                {
                    if(fdArray[i].fd == DFL)
                        break;
                }
                if(i == gnum) 
                {
                    cout << "服务器的同时连接数已达上限,请稍后再试." << endl;
                    close(connect);
                }
                else 
                {
                    cout << "获取到新连接,fd:" << connect << endl;   
                    fdArray[i].fd = connect; 
                    fdArray[i].events = POLLIN;
                }
            }
            else
            {
                Reader(fdArray[i]);
            }
        }
    }
}

void WriteHandler(fd_set* set)
{}

void ExceptHandler(fd_set* set)
{}

int main(int argc, char *argv[])
{
    // 1.创建套接字
    if (argc != 3 && argc != 2)
    {
        useage();
        exit(8);
    }
    string ip = argv[2] == nullptr ? "" : argv[2];
    uint16_t port = atoi(argv[1]);
    int listenFd = Sock::Socket();
    cout << "fd:" << listenFd << endl;
    // 2.绑定
    Sock::Bind(listenFd, ip, port);

    //3.创建监听套接字
    Sock::Listen(listenFd);
    // 4.监视"监听套接字"事件就绪,获取远端连接
    // 4.1 初始化文件描述符集
    for (int i = 0; i < gnum; ++i)
    {
        fdArray[i].fd = DFL;
    }
    fdArray[0].fd = listenFd;
    fdArray[0].events = POLLIN;//设置监听套接字的读事件
    ShowArray();
    while (true)
    {
        int timeout = 5000;
        int fdn = poll(fdArray, NUM, timeout);
        cout << "就绪事件数:" << fdn << endl;
        if (fdn > 0)
        {
            ReadHandler(listenFd);
            // WriteHandler(&writeFds);
            // ExceptHandler(&exceptFds);
        }
        else if(fdn == 0) 
        {
            cout << "没有就绪事件, 执行其他任务" << endl;
            work();
        }
        else
        {
            cerr << "poll Error, Code:" << errno << ":" << strerror(errno) << endl;
            exit(2);
        }
    }

    return 0;
}