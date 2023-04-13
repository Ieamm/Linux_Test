#include"Comm.hpp"

int main()
{
    //命名管道已经被服务端(读端)创建,直接打开
    int fd = Open(FIFO_PATHNAME,WRITER);

    //使用约定好的key值,获取在多个进程间可见的共享内存
    key_t key = create_key(PATHNAME,PROJ_ID);
    int shmid = shmget(key,MEM_SIZE,IPC_CREAT|0666);
    if(shmid == -1)
        printf("shmget error %s\n",strerror(errno));

    //关联共享内存
    char* ps = (char*)shmat(shmid,nullptr,0);
    
    //对共享内存写入
    printf("#Enter:");
    fflush(stdout);
    fgets(ps,MEM_SIZE,stdin);//从标准输入获取数据到ps
    //向负责访问控制的管道发出信号(向管道写入数据),表示共享内存中有数据可读.
    Signal(fd);

    //共享内存去关联
    shmdt(ps);

    //关闭管道文件以及递减文件引用计数
    Close(fd,FIFO_PATHNAME);
    return 0;
}