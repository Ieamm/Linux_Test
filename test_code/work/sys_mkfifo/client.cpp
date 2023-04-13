#include<cstdio>
#include<cstdlib>
#include<cstring>

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

int main()
{
    int fd = open("./fifo",O_RDWR);
    char buf[30];
    int len = sizeof(buf)/sizeof(char);
    memset(buf,'\0',len);
    read(fd,buf,len);
    printf("%s\n",buf);
    close(fd);
    unlink("./fifo");
    return 0;
}