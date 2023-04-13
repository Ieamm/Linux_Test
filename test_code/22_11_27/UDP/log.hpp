#pragma once
#include <iostream>
#include <cassert>
#include <cstdarg>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILEPATH "./server.log"
#define DEBUG 0
#define NOTICE 1
#define WARNING 2
#define FATAL 3

const char *log_level[] = {"DEBUG", "NOTICE", "WARNING", "FATAL"};

// 日志类
class Log
{
public:
    Log() : _logFd(-1)
    {
    }

    // 是否将日志内容写入日志
    void enbale()
    {
        umask(0);
        _logFd = open(FILEPATH, O_CREAT | O_WRONLY | O_RDONLY | O_APPEND, 0666);
        assert(_logFd != -1);
        // 重定向--改变标准输出和标准错误的文件描述符指向,将信息输入到文件中.
        dup2(_logFd, STDOUT_FILENO);
        dup2(_logFd, STDERR_FILENO);
    }
    ~Log()
    {
        if (_logFd != -1)
        {
            fsync(_logFd);
            close(_logFd);
        }
    }

private:
    int _logFd;
};

// logMessage(level,format,...)
void logMessage(int level, const char *format, ...)
{
    assert(level >= DEBUG);
    assert(level <= FATAL);

    char *name = getenv("USER");

    char logInfo[1024];
    va_list ap;
    va_start(ap, format);
    // 缓冲区大小-1,是为了写入'\0'
    vsnprintf(logInfo, sizeof(logInfo) - 1, format, ap);

    FILE *out = (level == FATAL) ? stderr : stdout;
    fprintf(out, "%s | %u | %s | %s",
            log_level[level],
            time(nullptr),
            name == nullptr ? "unknow" : name,
            logInfo);
    
    fflush(out);//将C缓冲区中的数据刷新到OS的内核缓冲区
    fsync(fileno(out));//使将内核缓冲区(磁盘缓冲区)中的数据尽快落盘
}