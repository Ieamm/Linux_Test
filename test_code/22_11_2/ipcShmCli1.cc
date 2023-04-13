#include"Comm.hpp"
using namespace std;
int main()
{
    int fd = Open(FIFO_FILE,WRITER);
    key_t Key = creatKey();
    log() << "进程:" << getpid() << " Create Key success | Key=" << "0x" << hex << Key << endl;

    //获取share memory id
    int shmid = shmget(Key,MEM_SIZE,IPC_CREAT);
    //关联
    char* str = (char*)shmat(shmid,nullptr,0);

    //TO DO
    while(true)
    {
        printf("Please Enter# ");
        fflush(stdout);
        ssize_t s = read(0,str,4396);
        if(s > 0)
        {
            //因为对于管道文件而言,我们的空格换行符也是一个字符,同样被写入到了管道内.
            str[s] = '\0'; 
        }
        Signal(fd);
    }

    // int count = 0;
    // while(count < 26)
    // {
    //     str[count] = 'A' + count;
    //     ++count;
    //     str[count] = '\0';
    //     sleep(1); 
    // }

    //去关联
    shmdt(str);

    //不需要进行删除,谁创建谁删除.
    return 0;
}