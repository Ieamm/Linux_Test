#pragma once
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<errno.h>
#include<assert.h>
#include<string>

#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#define PROJ_ID 15
#define PATHNAME "/home/yaotianyu/test_code/work/sys_shm"
#define MEM_SIZE 4096

key_t create_key(const char* pathname, int proj_id)
{
    key_t key = ftok(pathname,proj_id);
    if(key == -1)
    {
        printf("create_key error %s\n",strerror(errno));
        exit(1);
    }
    return key;
}

#define MODE 0777
#define FIFO_PATHNAME "./fifo"
//创建管道,用于共享内存的访问控制.
void create_fifo(const char* pathname, mode_t mode)
{
    int mr = mkfifo(pathname, mode);
    if(mr == -1)
    {
        printf("mkfifo error %s\n",strerror(errno));
    }
}

#define READER  O_RDONLY
#define WRITER  O_WRONLY
#define NONBLOCK O_NONBLOCK
int Open(const std::string &filename, int flags)
{
    int fd = open(filename.c_str(),flags);
    if(fd == -1)
    {
        printf("Open Fifo error %s\n",strerror(errno));
    }
    return fd;
}

void Signal(int fd)
{
    uint32_t cmd = 1;
    write(fd,&cmd,sizeof(cmd));
}

ssize_t Wait(int fd)
{
    uint32_t values = 0;
    ssize_t n = read(fd,&values,sizeof(values));
    return n;
}

void Close(int fd,const std::string filename)
{
    close(fd);
    unlink(filename.c_str());
}