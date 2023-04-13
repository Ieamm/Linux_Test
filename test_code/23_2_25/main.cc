#include"SetNonBlock.hpp"
#include<cstdio>

int count = 0;

void work()
{
    std::cout << "其他任务正在执行,当前:" << ++count << "项" << std::endl;
}

int main()
{
    char buffer[1024];
    SetNonBlock(0);//设置标准输入为非阻塞
    while(true)
    {
        buffer[0] = '\0'; //每次将buffer的首元素置0, 即切断该字符串.
        int n = scanf("%s",buffer);
        //观察标准输入变为非阻塞后,scanf的返回值
        std::cout << "scanf return:" << n << " Code:" << errno << ":" << strerror(errno) << std::endl;
        if(n > 0)//有输入
        {
            std::cout << "Enter# " << buffer << std::endl;
        }
        else
        {
            work();
        }
        sleep(1);
    }
    return 0;
}