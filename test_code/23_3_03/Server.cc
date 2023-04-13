#include "Sock.hpp"
#include "Connection.hpp"

#include <string>

#include <sys/select.h>
#include <unistd.h>

using namespace std;

#define DFL -1

const int NUM = (sizeof(fd_set) * 8);

Connection Connections[NUM];
const int gnum = sizeof(Connections) / sizeof(Connections[0]);

void ShowArray()
{
    for(int i = 0; i < gnum; ++i){
        if(Connections[i].fd != DFL)
            cout << "第" << i << "位的fd为:" << Connections[i].fd << endl;
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

void Excepter(Connection& conn);

int Reader(Connection &conn)
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

int Writer(Connection &conn)
{

}

void Excepter(Connection &conn)
{
    close(conn.fd);
    conn.fd = DFL;
}

// 4.事件就绪后进行处理
void ReadHandler(int listenSock, fd_set* set)
{
    for(int i = 0; i < gnum; ++i)
    {
        if(Connections[i].fd == DFL)  { continue; }
        
        if(FD_ISSET(Connections[i].fd, set))
        {
            //监听socket在获取新连接时,底层是要经过三次握手的，握手由Client端调用connect发起,由双方的传输层协议进行确认和连接建立。
            // 这里需要明确的是：网络协议栈本身存在于操作系统内部，是由操作系统内核通过软件的方式进行管理的。也就是说，TCP连接的实际建立，都是在内核中完成的。
            // accept的工作只是将已经在内核中被建立好, 存放在未决连接队列的TCP连接从内核空间拿到用户空间---本质就是数据拷贝的一种.
            //因此,对于底层有建立好的连接这一情况,我们可以称之为:读事件就绪!
            if(i == 0 && Connections[i].fd == listenSock)
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
                    if(Connections[i].fd == DFL)
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
                    Connections[i].fd = connect; 
                }
            }
            else
            {
                Reader(Connections[i]);
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
        Connections[i].fd = DFL;
    }
    Connections[0].fd = listenFd;
    fd_set readFds, writeFds, exceptFds;
    ShowArray();
    while (true)
    {
        // 4.2将输出参数清空
        FD_ZERO(&readFds);
        FD_ZERO(&writeFds);
        FD_ZERO(&exceptFds);
        // 4.3找到最大的文件描述符,并重新设置监听事件
        int maxFd = DFL;
        cout << "填充" << endl;
        for (int i = 0; i < gnum; ++i)
        {
            // 过滤不合法(未被设置)的位置
            if (Connections[i].fd == DFL)
                continue;

            // 将合法的fd的事件监听的打开--因为select的fd_set参数是输入输出参数,当作为输出参数时fd_set就会被重置,如果不在下一次检测时重新设置,就会导致前面获取到的需要监听的事件就绪的文件描述符丢失.
            // if (i > 0)
            // {
            //     FD_SET(Connections[i].fd, &writeFds);
            //     FD_SET(Connections[i].fd, &exceptFds);
            // }
            FD_SET(Connections[i].fd, &readFds);

            // 找出最大的fd.
            if (Connections[i].fd > maxFd)
                maxFd = Connections[i].fd;
        }
        struct timeval timeout = {5, 0};
        int fdn = select(maxFd + 1, &readFds, nullptr, nullptr, &timeout);
        cout << "就绪事件数:" << fdn << endl;
        if (fdn > 0)
        {
            ReadHandler(listenFd, &readFds);
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
            cerr << "select Error, Code:" << errno << ":" << strerror(errno) << endl;
            exit(2);
        }
    }

    return 0;
}