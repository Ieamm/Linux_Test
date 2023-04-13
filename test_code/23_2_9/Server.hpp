#include<iostream>
#include<string>
#include<stdlib.h>
#include<string.h>
#include<assert.h>

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<unistd.h>
class Server
{
public:
    Server(std::string i, uint16_t p):
    ip(i),
    port(p),
    listenFd(-1),
    quit(false)
    {}

    void init()
    {
        //创建套接字
        listenFd = socket(PF_INET, SOCK_STREAM, 0);
        if(listenFd < 0)
            std:: cerr << "socket create fail" << std::endl;

        // 服务崩溃后立刻重启
        int opt = 1;
        setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(listenFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

        //绑定信息到内核
        struct sockaddr_in own;
        int len = sizeof(own);
        memset(&own, 0, len);
        own.sin_family = AF_INET;
        if(ip.empty())
        { own.sin_addr.s_addr = INADDR_ANY; }
        else if(inet_aton(ip.c_str(), &own.sin_addr) == 0)
        { std::cerr << "ip format unvalid" << std::endl; }
        own.sin_port = htons(port);
        bind(listenFd, (const sockaddr*)&own, len);

        //创建监听套接字
        listen(listenFd, 1);
    }

    void loop()
    {
        while(!quit)
        {
            //接收对端主机信息
            struct sockaddr_in peer;
            socklen_t n = sizeof(peer);
            std::cout << "等待对端连接" << std::endl;
            int serviceSock = accept(listenFd, (sockaddr*)&peer, &n);
            if(quit)
                return ;
            if(serviceSock < 0)
            { std::cerr << "serviceSock create fail." << std::endl; }; 

            std::cout << "serviceSock create success.";           
        }
    }
private:
    std::string ip;
    uint16_t port;
    int listenFd;
    bool quit;
};