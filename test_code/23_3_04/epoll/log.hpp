#pragma once

#include<iostream>
#include<cstring>
#include<cassert>
#include<cstdarg>

#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>


#define DEBUG 0
#define NOTICE 1
#define WARNING 2
#define FATAL 3

#define LOG_FILE "tcpServer.log"

class Log
{
public:
    Log():_logFd(-1)
    {}
    ~Log()
    {
        if(_logFd != -1)
        {
            fsync(_logFd);//使得文件缓冲区内的内容所占的磁盘缓冲区尽快落盘
            close(_logFd);//关闭文件描述符
        }
    }
public:
    void enable()
    {
        umask(0);
        _logFd = open(LOG_FILE,O_APPEND | O_CREAT | O_WRONLY | O_RDONLY,0666);
        assert(_logFd != -1);
        //将标准输出和标准错误重定向到我们上面打开的日志文件中.
        dup2(_logFd,STDOUT_FILENO);
        dup2(_logFd,STDERR_FILENO);
    }
private:
    int _logFd;
};

const char* levelSet[] = {"DEBUG","NOTICE","WARNING","FATAL"};
void logMessage(int level,const char* format, ...)
{
    assert(level >= DEBUG);
    assert(level <= FATAL);
    //标识日志生成者的身份.
    char* name = getenv("USER");
    //解析可变参数的缓冲区
    char logInfo[1024];
    
    //解析可变参数
    va_list ap;
    va_start(ap,format);
    //将可变参数的解析结果还原到指定缓冲区--logInfo内
    vsnprintf(logInfo, sizeof(logInfo)-1, format, ap);

    FILE *out = (level == FATAL ? stderr : stdout);
    fprintf(out, "%s | %ld | %s | %s",
            levelSet[level], time(nullptr),
            name == nullptr ? "unknow" : name,
            logInfo);
    
    fflush(out);//将文件缓冲区内的数据刷新到对应的流文件中
    fsync(out->_fileno);//将文件中暂存在磁盘缓冲区内,还未落盘的数据尽快落盘.
}

