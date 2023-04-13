#include"Comm.hpp"


int main()
{
    //创建命名管道,用以对共享内存实现访问控制
    create_fifo(FIFO_PATHNAME,MODE);
    //打开命名管道
    int fd = Open(FIFO_PATHNAME,READER);

    //使用约定好的key来获取共享内存,若不存在则创建.
    key_t key = create_key(PATHNAME,PROJ_ID);
    int shmid = shmget(key,MEM_SIZE,IPC_CREAT|0666);
    if(shmid == -1)
        printf("shmget error %s\n",strerror(errno));
    
    //将共享内存与进程之间进行关联,并返回共享内存的地址
    char* ps = (char*)shmat(shmid,nullptr,0);
    //等待写端进程对共享内存进行写入.
    int n = Wait(fd);
    if(n > 0)//代表写端未被关闭,且当前管道内有内容需要读取(也就代表共享内存中有需要继续读取的内容)
    {
        printf("%s\n",ps);
    }

    //去除共享内存与当前进程的关联
    shmdt(ps);

    //删除共享内存
    shmctl(shmid,IPC_RMID,nullptr);

    //关闭对共享内存进行访问控制的管道文件端,同时使其引用计数--,引用计数为0时删除该文件.
    Close(fd,FIFO_PATHNAME);

    return 0;
}