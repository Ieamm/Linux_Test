#include"comm.h"

int main()
{
    //先要确保有管道文件存在,因此先创建一个命名管道文件
    int is_f = mkfifo(IPC_PATH,0600);
    if(is_f == -1)//管道文件创建失败
    {
        cerr << "mkfifo error: " <<  strerror(errno) << endl;
    }
    
    //确保管道文件存在后,将其以读方式打开
    int pipeFd = open(IPC_PATH,O_RDONLY);
    if(pipeFd < 0)//打开文件失败
    {
        cerr << "open error: " << strerror(errno) << endl;
    }

    #define NUM 1024
    char strbuffer[NUM];
    while(true)
    {        
        memset(strbuffer, 0, sizeof(strbuffer));
        //strbuffer的大小-1,留出\0的位置.
        ssize_t ret = read(pipeFd, strbuffer, sizeof(strbuffer)-1);
        //除了这个读取端口以外,管道文件的其他端口都被关闭了,且这个读取端口也读取到了最后,即文件读取结束
        if(ret == 0)
        {
            cout << "客户端发送完毕,服务端即将关闭." << endl;
            break;
        }
        else if(ret > 0)//读取管道缓冲区内的数据成功,返回字节数
        {
            //为字符串的结尾附上'\0'
            strbuffer[ret] = '\0';
            cout << "客户端->服务端#" <<strbuffer << endl;
        }
        else 
        {
            cerr << "read error: " << strerror(errno) <<  endl;
            break;
        }
    }
    
    //服务正常结束
    close(pipeFd);
    cout << "服务端关闭." << endl;
    unlink(IPC_PATH);

    return 0;
}

