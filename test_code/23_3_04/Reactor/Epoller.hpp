#include "log.hpp"
#include<sys/epoll.h>

class Epoller
{
    static const int gsize = 128;
public:
    static int CreateEpoll()
    {
        int epfd = epoll_create(gsize);
        if(epfd < 0)
        {
            logMessage(FATAL,"CreateEpoll Error\n");
        }

        return epfd;
    }

    static bool AddConnection(int epfd, int sock, uint32_t event)
    {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = sock;
        int n = epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);

        return n == 0;
    }

    static bool DelConnection(int epfd, int sock)
    {
        int n = epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr);

        return n == 0;
    }

    static bool ModConnection(int epfd, int sock, uint32_t event)
    {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = sock;

        int n = epoll_ctl(epfd, EPOLL_CTL_MOD, sock, &ev);
        return n == 0;
    }

    static int loopOnce(int epfd, struct epoll_event revs[], int revs_num)
    {
        int timeout = 4000;
        int n = epoll_wait(epfd, revs, revs_num, timeout);
        if(n == -1)
            logMessage(NOTICE, "waiting for events to be ready--Error");
        
        return n;
    }    
};