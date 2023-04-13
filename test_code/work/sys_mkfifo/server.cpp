#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<errno.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>

void init()
{
    int mr = mkfifo("./fifo",0777);
    if(mr == -1)
    {
        printf("mkfifo fail : %s\n",strerror(errno));
    }
}

int main()
{
    init();
    FILE* pf = fopen("./fifo","w");
    const char* msg = "i am process A";
    fwrite(msg,strlen(msg),sizeof(char),pf);
    fclose(pf);
    unlink("./fifo");
    return 0;
}