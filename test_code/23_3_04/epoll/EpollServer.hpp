#include "Sock.hpp"
#include "log.hpp"

#include <string>
#include <functional>
#include <iostream>

#include <sys/epoll.h>
#include <poll.h>
#include <unistd.h>

#define DFL -1

using func_t = std::function<int(int)>;
const int gsize = 256;
const int num = 1024;

class EpollServer
{
public:
    EpollServer(uint16_t port, func_t func, std::string ip = "") : port_(port),
                                                              func_(func),
                                                              listenSock_(-1),
                                                              epfd_(-1)
    {
    }
    ~EpollServer()
    {
        if(listenSock_ == -1)
        {
            close(listenSock_);
        }
        if(epfd_ == -1)
        {
            close(epfd_);
        }
    }
    void initEpoll()
    {
        epfd_ = epoll_create(gsize);
        if (epfd_ < 0)
        {
            logMessage(FATAL, "epoll_create error\n");
            exit(3);
        }
        listenSock_ = Sock::Socket();
        if (listenSock_ < 0)
        {
            logMessage(FATAL, "listenSock create error\n");
            exit(3);
        }
        Sock::Bind(listenSock_, ip_, port_);

        Sock::Listen(listenSock_);
    }

    void handlerEvents(struct epoll_event revents[], int n)
    {
        for (int i = 0; i < n; ++i)
        {
            if (revents[i].events == EPOLLIN)
            {
                if (revents[i].data.fd == listenSock_) // 底层连接就绪
                {
                    uint16_t peer_Port = 0;
                    std::string peer_ip = "";
                    int service = Sock::Accept(listenSock_, &peer_ip, &peer_Port);
                    if (service < 0)
                    {
                        std::cout << "Accpet Error" << strerror(errno) << std::endl;
                        return;
                    }

                    struct epoll_event service_ev;
                    service_ev.data.fd = service;
                    service_ev.events = EPOLLIN;
                    int n = epoll_ctl(epfd_, EPOLL_CTL_ADD, service, &service_ev);
                    assert(n == 0);
                    (void)n;
                }
                else
                {
                    int n = func_(revents[i].data.fd);
                    if(n < 0)
                    {
                        logMessage(WARNING, "Read Error,%d,%s\n", errno, strerror(errno));
                        int x = epoll_ctl(epfd_, EPOLL_CTL_DEL, revents[i].data.fd, nullptr);
                        assert(x == 0);
                        (void)x;
                        close(revents[i].data.fd);//因为epoll_ctl是对一个合法的文件描述符进行控制, 因此对于关闭次序也需要注意
                    }
                }
            }
        }
    }

    void run()
    {
        // 1.将listensock添加进接收数组中
        struct epoll_event listenSock_Ev;
        listenSock_Ev.events = EPOLLIN; // 设置读事件监听
        listenSock_Ev.data.fd = listenSock_;
        if (epoll_ctl(epfd_, EPOLL_CTL_ADD, listenSock_, &listenSock_Ev) < 0)
        {
            logMessage(WARNING, "listenSock epoll add error\n");
            exit(3);
        }

        struct epoll_event revs[num]; // 用以接收就绪队列中的就绪节点
        int timeout = 10000;
        while (true)
        {
            int n = epoll_wait(epfd_, revs, num, timeout);
            switch (n) // 接收到了就绪的fd
            {
            case 0:
                std::cout << "time out..." << (unsigned long)time(nullptr) << std::endl;
                break;
            case -1:
            {
                std::cout << "errno:" << errno << " error details:" << strerror(errno) << std::endl;
            }
            break;
            default:
                handlerEvents(revs, n); // 这里要注意传参问题:revs是存储就绪队列内已经就绪的fd的结构,因此并不需要将整个容器遍历,只需要遍历目前revs内存储的就绪fd即可.而就绪fd的数量就是epoll_wait返回的大于0的值
                break;
            }
        }
    }

private:
    int listenSock_;
    int epfd_;
    uint16_t port_;
    std::string ip_;
    func_t func_;
};