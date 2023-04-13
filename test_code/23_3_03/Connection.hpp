#pragma once
#include<string>

#define SIZE 1024

struct Connection
{
public:
    int fd;
    std::string inBuffer;
    std::string outBuffer;
};