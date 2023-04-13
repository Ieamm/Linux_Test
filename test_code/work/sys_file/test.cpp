#include<cstring>
#include<cstdio>
#include<cstdlib>

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>


int main()
{
    //以读写方式打开文件,若文件不存在则创建,且每次写入前清空文件,文件权限设置为0777-->默认权限掩码0002
    int fd = open("./bite",O_CREAT | O_RDWR | O_TRUNC, 0777);
    if(fd == -1)
    {
        perror("oepn fail!\n");
        exit(1);
    }
    
    const char* msg = "I like Linux!";
    size_t size = strlen(msg);
    //将msg向文件写入
    write(fd,msg,size);
    

    char buf[20];
    size_t len = sizeof(buf)/sizeof(char);
    memset(buf,'\0',len);
    //在发生写入后,当前fd所指向的文件的文件指针来到了文件末尾,因此需要使用lseek接口将文件指针还原到文件起始位置.
    lseek(fd,0,SEEK_SET);
    size_t n = read(fd,buf,len);//读取buf大小的数据
    if(n == -1)//读取失败
    {
        perror("read fail!");
        exit(1);
    }
    
    //向标准输出打印
    fprintf(stdout,"从文件描述符:%d 中读取的数据为:%s\n",fd,buf);
    close(fd);//关闭文件
    return 0;
}