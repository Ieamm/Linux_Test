#include "Epoller.hpp"
#include "log.hpp"
#include "Sock.hpp"
#include "Util.hpp"
#include "protocol.hpp"
#include <functional>
#include<unordered_map>
#include<string>
#include<vector>

class TcpServer;
class Connection;

using callback_t = std::function<int(Connection*, std::string&)>;
using func_t = std::function<int(Connection*)>;

//链接对象
class Connection
{
public:
    int fd_;
    std::string inbuffer_;
    std::string outbuffer_;
    TcpServer* R_; //回指链接对象存在的服务器
    func_t recver_;
    func_t sender_;
    func_t excepter_;

public:
    Connection(int fd, TcpServer* R):fd_(fd), R_(R), recver_(nullptr), sender_(nullptr), excepter_(nullptr)
    {}
    
    void setRecver(func_t recver)
    {recver_ = recver; }
    void setSender(func_t sender)
    {sender_ = sender; }
    void setExcepter(func_t excepter)
    {excepter_ = excepter; }
};


class TcpServer
{
public:
    TcpServer(callback_t cb, int port = 8080):cb_(cb)
    {
        //创建套接字
        listenSock_ = Sock::Socket();
        //绑定套接字
        Sock::Bind(listenSock_, "", port);
        //套接字监听
        Sock::Listen(listenSock_);
        //设置监听套接字非阻塞 -- 为了适配ET模式
        SetNonBlock(listenSock_);
        //分配内存给接收就绪事件的数组
        revs_ = new struct epoll_event[revs_num];
        
        //创建Epoll模型
        epfd_ = Epoller::CreateEpoll();
        
        //因为这些读写处理方法都是类内成员, 还需要有一个隐式this指针进行调用,使用bind将其绑定.
        AddConnection(listenSock_, EPOLLIN | EPOLLET, 
                        std::bind(Accept, this, std::placeholders::_1), nullptr, nullptr);
    }

    ~TcpServer()
    {}
    
    //将套接字封装进Connection对象,并将其添加到epoll模型中,再放入Connections中集合管理
    void AddConnection(int sock, uint32_t event, func_t recver, func_t sender, func_t excepter)
    {
        Connection* conn = new Connection(sock, this);
        if(recver)
        { conn->setRecver(recver); }
        if(sender)
        { conn->setSender(sender); }
        if(excepter)
        { conn->setExcepter(excepter); } 

        Epoller::AddConnection(epfd_, sock, event);

        Connections_.insert(std::make_pair(sock, conn)); 
        logMessage(DEBUG, "获取新连接到connections中成功--socket:%d\n",sock);
    }

    void EnableReadWrite(int sock, bool readable, bool writeable)
    {
        uint32_t event = 0;
        event |= readable ? EPOLLIN : 0;
        event |= writeable ? EPOLLOUT : 0;
        Epoller::ModConnection(epfd_, sock, event);
    }

    int TcpSender(Connection* conn)
    {
        while(true)
        {
            int n = send(conn->fd_, conn->outbuffer_.c_str(), conn->outbuffer_.size(), 0);

            if(n > 0)
            {
                //将已经发送的数据删除.
                conn->outbuffer_.erase(0,n);
            }
            else
            {
                if(errno == EINTR) { continue; }    //本次发送操作被信号阻断
                else if(errno == EWOULDBLOCK || errno == EAGAIN) { break; } //表示发送结束 -- 但却不一定是发送缓冲区为空.可能是对端的接收缓冲区被写满,但数据还未发送完毕.
                else//发送出错
                {
                    conn->excepter_(conn);
                    logMessage(WARNING, "TcpSender error sock:%d\n", conn->fd_);
                    return -1;
                }
            }
        }

        return 0;
    }

    int TcpRecver(Connection* conn)
    {
        while(true)
        {
           char message[1024];
           ssize_t s = recv(conn->fd_, message, sizeof(message) / sizeof(message[0]), 0);
           if(s > 0)
           {
                conn->inbuffer_ += message;
           }
           else
            {
                if(errno == EINTR) { continue; }    //本次读取操作被信号阻断
                else if(errno == EWOULDBLOCK || errno == EAGAIN) { break; } //表示读取完毕 -- 不代表接收缓冲区内没有数据了,但因为我们对读事件在epoll模型中设置了常关心,等待下一次就绪
                else//读取出错
                {
                    conn->excepter_(conn);
                    logMessage(WARNING, "TcpRecver error sock:%d\n", conn->fd_);
                    return -1;
                }
            }
        }

        //将接收到的数据拆分成完整报文
        std::vector<std::string> result;
        Protocol::PackageSplit(conn->inbuffer_, result);
        for(auto message : result)
        {
            cb_(conn, message);
        }
        return 0;
    }

    int TcpExcepter(Connection* conn)
    {
        if(IsExists(conn->fd_))
            return -1;

        //从epoll模型中移除
        Epoller::DelConnection(epfd_, conn->fd_);
        logMessage(NOTICE, "将sock[%d]从epoll模型中移除\n", conn->fd_);

        //关闭文件描述符
        close(conn->fd_);
        logMessage(NOTICE, "close sock[%d]\n",conn->fd_);

        //释放资源
        delete Connections_[conn->fd_];

        //从Connections中移除
        Connections_.erase(conn->fd_);
        logMessage(NOTICE, "将sock[%d]从Connections中移除\n", conn->fd_);
        
    }

    //listenSock的读方法 -- 将就绪队列中就绪的连接读到用户空间
    int Accept(Connection* conn)
    {
        //ET模式下, 当就绪队列中数据抵达时,epoll_wait只会提醒一次.即使该数据没有被读取完毕,也不会再向上层发送提醒.
        //对于就绪的连接也是一样,如果同一时间抵达大量连接,却又没有读取完毕,就会导致连接丢失.
        //因此,ET模式下对就绪事件进行轮询检测,只要就绪条件还满足就一直进行处理,防止数据丢失.
        while(true)
        {
            std::string clientIp;                                        
            uint16_t clientPort;
            //对accept的调用不会被阻塞
            int sock = Sock::Accept(listenSock_, &clientIp, &clientPort);
            if(sock < 0)
            {
                if(errno == EINTR) //因异步信号的产生而错误
                    continue;
                else if(errno == EAGAIN || errno == EWOULDBLOCK)//流已空
                    break;
                else
                {
                    logMessage(WARNING, "Tcp::Accept error, sock:%d\n", sock);
                    return -1;
                }
            }
            logMessage(DEBUG, "get a new link --- srcIp:%s srcPort:%d sock:%d\n", \
                        clientIp.c_str(), clientPort, sock);
            
            //常打开读事件关心,需要时再打开写事件关心.
            AddConnection(sock, EPOLLIN | EPOLLET, 
                        std::bind(TcpRecver, this, conn), 
                        std::bind(TcpSender, this, conn),
                        std::bind(TcpExcepter, this, conn));
        }

        return 0;

    }

    bool IsExists(int sock)
    {
        return Connections_.find(sock) != Connections_.end();
    }

    //分配器---派发就绪任务
    void Dispatcher()
    {
        int n = Epoller::loopOnce(epfd_, revs_, revs_num);
        for(int i = 0; i < n; ++i)
        {
            uint32_t revent = revs_[i].events;
            int sock = revs_[i].data.fd;

            //关心的文件描述符出现错误时 --- 因为我们将异常管理在读写操作中实现, 因此我们可以对将出错的文件描述符的异常也一同放在读写操作中处理 -- 将它们赋值为读写事件,因为是出错的文件描述符,必然不会正常的完成任务,异常字段也是辨认手段.
            if(revent == EPOLLHUP) revent |= EPOLLIN;
            if(revent == EPOLLERR) revent |= EPOLLOUT;

            if(revent == EPOLLIN)
            {
                if(IsExists(sock) && Connections_[sock]->recver_)
                {
                    Connections_[sock]->recver_(Connections_[sock]);
                }
            }
            
            if(revent == EPOLLOUT)
            {
                if(IsExists(sock) && Connections_[sock]->sender_)
                {
                    Connections_[sock]->sender_(Connections_[sock]);
                }
            }
        }
    }

    void Run()
    {
        while(true)
        {
            Dispatcher();
        }
    }

private:
    static const int revs_num = 64;
    //1.网络监听套接字
    int listenSock_;
    //2.将epoll与上层代码进行结合
    std::unordered_map<int, Connection*> Connections_;
    //3.就绪事件列表
    struct epoll_event *revs_;
    //4.报文处理回调函数
    callback_t cb_;
    //5.epoll模型
    int epfd_;
};
