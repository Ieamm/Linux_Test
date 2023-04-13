#include<unistd.h>
#include<fcntl.h>
#include<iostream>
#include<cstring>

static void SetNonBlock(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    int n = fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    if(n == -1)
        std::cout << "Set NonBlock error, Error Code" << errno << strerror(errno);
}