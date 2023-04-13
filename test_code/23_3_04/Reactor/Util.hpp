#pragma once
#include<iostream>
#include<cstring>
#include<unistd.h>
#include<fcntl.h>


void SetNonBlock(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    int n = fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    if(n < 0)
    {
        std::cout << "SetNonBlock Error : " << strerror(errno) << std::endl;
    }
}

