#pragma once

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Log.hpp"

#define PATH_NAME "/home/yaotianyu/test_code/11_2"
#define PROJ_ID 0x14
#define MEM_SIZE 4096

#define FIFO_FILE ".fifo"

key_t CreateKey()
{
    key_t key = ftok(PATH_NAME, PROJ_ID);
    if(key < 0)
    {
        std::cerr <<"ftok: "<< strerror(errno) << std::endl;
        exit(1);
    }
    return key;
}

void CreateFifo()
{
    umask(0);
    if(mkfifo(FIFO_FILE, 0666) < 0)
    {
        Log() << strerror(errno) << "\n";
        exit(2);
    }
}

#define READER O_RDONLY
#define WRITER O_WRONLY

int Open(const std::string &filename, int flags)
{
    return open(filename.c_str(), flags);
}

int Wait(int fd)
{
    uint32_t values = 0;
    ssize_t s = read(fd, &values, sizeof(values));
    return s;
}

void Signal(int fd)
{
    uint32_t cmd = 1;
    write(fd, &cmd, sizeof(cmd));
}

void Close(int fd, const std::string filename)
{
    close(fd);
    unlink(filename.c_str());
}
