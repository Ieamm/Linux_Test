#include<cstdarg>
#include<cstdio>
#include<cstdint>
#include<string>
#include<string.h>

#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

struct ip
{
    uint32_t part1:8; uint32_t part2:8; uint32_t part3:8; uint32_t part4:8;
};

int main()
{
    // char s[] = "194.21.49.01";
    // char* sf = s;
    // struct ip _ip;
    // char* se = strtok(s,".");
    // while(se != NULL)
    // {
    //     int count = 0;
    //     int n = 0;
    //     for(sf; sf!=se; ++sf)//根据指针类型char*,得知每个指向物的大小为1字节,sf每次走1字节
    //     {
    //         n *= 10;
    //         n += *sf - '0';      
    //     }
    //     count++;
    //     se = strtok(NULL,".");
    //     if(count == 1)
    //         _ip.part1 = n;
    //     else if(count == 2)
    //         _ip.part2 = n;
    //     else if(count == 3)
    //         _ip.part3 = n;
    //     else
    //         _ip.part4 = n;
    // }
    
    // //...

    STDERR_FILENO;
    stdin;
    INADDR_ANY;

    return 0;
}

