#include"comm.h"

int main()
{
    //对于已经存在的命名管道文件,就没必要再创建一次了.

    int pipeFd = open(IPC_PATH,O_WRONLY);
    if(pipeFd < 0)
    {
        cerr << "open error" << strerror(errno) << endl;
        return 1;
    }
    
    #define NUM 1024
    char strbuffer[1024];
    while(true)
    {
        printf("请输入你的消息# ");
        fflush(stdout);
        memset(strbuffer,0,sizeof(strbuffer));
        
        //从标准输入中获取用户输入
        if(fgets(strbuffer,sizeof(strbuffer),stdin) != nullptr)
        {
            //这里-1是因为我们输入的信息是以换行符结尾,即:"abcdef\n\0"
            //而在文本中这个\n实际上不应该存在,因为这是C语言的转义字符,而不是计算机字符的转义字符.
            //因此,将这个转义字符改为'\0'.
            strbuffer[strlen(strbuffer) - 1] = '\0';
            write(pipeFd, strbuffer, strlen(strbuffer));
        }
        else
        {
            cerr << "fgets error" << endl;
            break;
        }
    }
    
    //这里不需要unlink,因为当我们关闭写端时,管道文件内的缓冲区中可能还有数据正在被读取.
    close(pipeFd);
    cout << "客户端退出" << endl;   
    return 0;
}