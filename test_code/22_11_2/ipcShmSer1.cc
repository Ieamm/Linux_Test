#include "Comm.hpp"
#include "log.hpp"

#include <unistd.h>

using namespace std;

// 我想创建全新的共享内存
const int flags = IPC_CREAT | IPC_EXCL;

// 充当使用共享内存的角色
int main()
{
    CreateFifo();
    int fd = Open(FIFO_FILE, READER);
    assert(fd >= 0);


    key_t key = creatKey();
    log() << "key: " << key << "\n";
    log() << "create share memory begin\n";
    int shmid = shmget(key, MEM_SIZE, flags | 0666);

    if (shmid < 0)
    {
        log() << "shmget: " << strerror(errno) << "\n";
        return 2;
    }
    log() << "create shm success, shmid: " << shmid << "\n";
    // sleep(5);

    // 1. 将共享内存和自己的进程产生关联attach
    char *str = (char *)shmat(shmid, nullptr, 0);
    log() << "attach shm : " << shmid << " success\n";

    // sleep(5);
    // 用它
    while(true)
    {
        // 让读端进行等待
        if(Wait(fd) <= 0) break; 
        printf("%s\n", str);
        sleep(1);
    }

    // 2. 去关联
    shmdt(str);
    log() << "detach shm : " << shmid << " success\n";
    // sleep(5);

    // 删它
    shmctl(shmid, IPC_RMID, nullptr);

    log() << "delete shm : " << shmid << " success\n";

    Close(fd, FIFO_FILE);
    // sleep(5);
    return 0;
}