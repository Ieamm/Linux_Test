#include<iostream>
#include<cstring>

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

struct Sock
{
    static int Socket()
    {
        int fd = socket(PF_INET, SOCK_STREAM, 0);
        if(fd < 0)
            std::cout << "Socket Create Error" << strerror(errno) <<std::endl;

        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
        return fd;
    }

    static void Listen(int fd)
    {
        if(listen(fd, 3) < 0)
            std::cout << "Listen Error" << strerror(errno) << std::endl;
    }

    static void Bind(int fd, std::string ip, uint16_t port)
    {
        struct sockaddr_in local;
        int n = sizeof(local);
        memset(&local, 0, n);
        local.sin_port = htons(port);
        local.sin_family = AF_INET;
        if(ip == "")
            local.sin_addr.s_addr = INADDR_ANY;
        else
            if(inet_aton(ip.c_str(), &local.sin_addr) == 0)
            {
                std::cout << "Ip format invalid: " << ip << std::endl;
                exit(2);
            }

        int r = bind(fd, (const struct sockaddr*)&local, n); 
        if(r < 0)
        {
            std::cout << "Bind Error" << strerror(errno) <<std::endl;
            exit(2);
        }
    }

    static int Accept(int listenFd, std::string* ip, uint16_t* port)
    {
        struct sockaddr_in peer;
        socklen_t n = sizeof(peer);
        memset(&peer, 0, n);
        int serviceFd = accept(listenFd, (struct sockaddr*)&peer, &n);
        if(serviceFd < 0)
            std::cout << "Accpet Error" << strerror(errno) <<std::endl;
        
        if(ip)   *ip = inet_ntoa(peer.sin_addr);
        if(port) *port = ntohs(peer.sin_port);
        
        return serviceFd;
    }
};