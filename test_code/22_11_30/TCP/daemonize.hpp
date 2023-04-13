#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<signal.h>
#include<cassert>
#include"log.hpp"
void daemonize()
{
    int id;

    //忽略SIG_PIPE信号.
    //避免因:将客户读端关闭后,服务端再对其写入,返回的SIG_PIPE信号而意外终止的情况
    signal(13,SIG_IGN);
    
    //2.更改进程的工作目录--这里选择不更改.
    //chdir()
    
    //3.不要让自己成为组长
    if((id = fork()) > 0)
    {
        exit(1);
    }
    
    setsid();

    if(id == 0)//子进程
    {
        //4.将该子进程设置为一个独立的会话以及进程组
        int dustbin_fd = open("/dev/null",O_RDWR);
        logMessage(DEBUG,"%d",dustbin_fd);
        assert(dustbin_fd >= 0);
        dup2(dustbin_fd,STDOUT_FILENO);
        dup2(dustbin_fd,STDERR_FILENO);
        dup2(dustbin_fd,STDIN_FILENO);

        //5关闭不需要的文件描述符
        if(dustbin_fd > STDERR_FILENO)
        {
            close(dustbin_fd);
        }
    }
}