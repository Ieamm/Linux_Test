#pragma once
#include<iostream>
#include<cstring>
#include<cerrno>
#include<cstdio>
#include<cassert>
#include<string>

#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>

#include"log.hpp"

//创建端和使用端约定使用同一个路径和指定值
#define PATHNAME "/home/yaotianyu/test_code/11_2"
#define PROJID 0x20
//希望创建的共享内存大小
const int MEM_SIZE = 4396;//默认为4KB

//命名管道所在路径
#define FIFO_FILE ".fifo"


key_t creatKey()
{
    int ret = ftok(PATHNAME,PROJID);
    if(ret < 0)
    {
        std::cerr << "ftok" << strerror(errno) << std::endl;
        exit(1);//若Key_t值创建失败,则后续代码无需执行,直接结束进程.
    }
    
    return ret;
}

///使用管道实现对共享内存的访问控制///

//创建管道文件
void CreateFifo()
{
    umask(0);
    if(mkfifo(FIFO_FILE, 0666) < 0)
    {
        log() << strerror(errno) << "\n";
        exit(2);
    }
}

//打开管道文件
#define READER O_RDONLY
#define WRITER O_WRONLY
int Open(const std::string &filename,int flag)
{
    return open(filename.c_str(),flag);
}

//向管道写入表示读取端可以进行读取
void Signal(int fd)
{
    int num = 1;
    ssize_t s = write(fd,&num,sizeof(num));
    if(s < 0)
    {
        log() << "Signal send error | " << strerror(errno) << std::endl;
    }
}

//读取端等待管道写入,表示写入端进行写入
int Wait(int fd)
{
    int num = 0;
    ssize_t s = read(fd,&num,sizeof(num));
    return  s;
}

//关闭管道文件
void Close(int fd,const std::string filename)
{
    close(fd);
    unlink(filename.c_str());
}