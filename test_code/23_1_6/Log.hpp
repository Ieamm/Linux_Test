#pragma once
#include"util.hpp"

#include<cstdio>
#include<cstdarg>
#include<cassert>
#include<cstdlib>
#include<ctime>

#ifdef SERVER
#define LOGFILE "Server.log"
#else
#define LOGFILE "Client.log"
#endif

class Log
{
public:
    Log(): logFd(-1)
    {}
    ~Log()
    {
        if(logFd != -1)
        {
            fsync(logFd);
            close(logFd);
        }
    }
public:
    void enable()
    {
        umask(0);
        logFd = open(LOGFILE, O_CREAT | O_APPEND | O_WRONLY | O_RDONLY, 0666);
        assert(logFd != -1);
        dup2(logFd, STDERR_FILENO);
        dup2(logFd, STDOUT_FILENO);
    }
private:
    int logFd;
};

#define DEBUG 0
#define NOTICE 1
#define WARNING 2
#define FATAL 3

const char* levels[] = {"DEBUG", "NOTICE", "WARNING", "FATAL"};
void logMessage(size_t level, const char* format, ...)
{
    assert(level >= 0 && level <= 3);

    const char* name = getenv("USER");
    name == nullptr ? "unknow" : name;

    va_list ap;
    va_start(ap, format);//格式化解析可变参数列表
    char logInfo[1024];
    vsnprintf(logInfo, sizeof(logInfo)-1, format, ap);

    FILE *out = level == FATAL ? stderr : stdout;
    fprintf(out, "%s | %d | %s | %s", levels[level], time(nullptr), name, logInfo);
    fflush(out);
    fsync(out->_fileno);
}


