#include<cstring>
#include<cstdlib>
#include<iostream>
#include<cstdio>

#include<unistd.h>
using namespace std;
int main()
{
    int fds[2];
    int pr = pipe(fds);//返回打开的管道文件的读写两端的文件描述符在这个数组里,下标0为读,1为写.
    if(pr == -1)
    {
        cout << "pipe error message: " << strerror(errno) << endl;
    }
    
    pid_t id = fork();
    if(id == 0)//子进程
    {
        //子进程为读端,因此关闭其写端.
        close(fds[1]);
        char buf[20];
        int len = sizeof(buf)/sizeof(char);
        memset(buf,'\0',len);
        size_t n = read(fds[0],buf,len);//阻塞等待,因此若管道缓冲区内没有数据,则一直阻塞在这里.
        if(n==0)
            cout << "所有写端都已经被关闭" << endl;
        fprintf(stdout,"%s\n",buf);//向标准输出打印
        //关闭读端
        close(fds[0]);
    }
    else if(id == -1)//子进程创建失败
    {
        cout << "fork error message: " << strerror(errno) << endl;
    }
    else//父进程
    {
        //父进程为写端,因此关闭其读端.
        close(fds[0]);
        const char* msg = "i am father";
        write(fds[1],msg,strlen(msg));
        close(fds[1]);
    }
    return 0;
}