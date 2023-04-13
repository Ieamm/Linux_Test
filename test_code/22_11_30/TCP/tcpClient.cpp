#include"log.hpp"
#include"util.hpp"
#include<iostream>
#include<cstdarg>

//思考一下:
//a.对于tcp客户端而言:需要将[ip:port] bind到内核中等待查询吗? 需要,当然需要! 但我们不需要自己来bind.
//b.需要listen吗? 不需要!我们是发起连接请求的一方!
//c.需要accept吗? 如上,也不需要!


class tcpClient
{
public:
    tcpClient(uint16_t port, std::string ip) : _servicePort(port), _serviceIp(ip), _sockFd(-1), _quit(false)
    {}
    ~tcpClient()
    {}

public:
    void init()
    {
        //1.创建tcp套接字
        _sockFd = socket(AF_INET,SOCK_STREAM,0);
        if(_sockFd == -1)
        {
            logMessage(FATAL,"socket error : %s %d",strerror(errno),_sockFd);
            exit(1);
        }
        logMessage(DEBUG,"socket success:%d\n",_sockFd);
        //2.connect,向远端服务发起连接请求.
        //2.1 填充客户端的基本信息
        struct sockaddr_in service;
        socklen_t len = sizeof(service);
        memset(&service,0,len);
        service.sin_family = AF_INET;
        service.sin_port = htons(_servicePort);
        if(inet_aton(_serviceIp.c_str(),&service.sin_addr) == 0)
        {
            logMessage(FATAL,"Service ip invalid : %s %d\n",_serviceIp.c_str(),_sockFd);
            exit(2);
        } 
        //2.2发起请求,connect会自动将我们的[ip:port]绑定
        if(connect(_sockFd,(const sockaddr*)&service,len) == -1)
        {
            logMessage(FATAL,"connect error : %s %d\n",strerror(errno),_sockFd);
        }
        logMessage(DEBUG,"connect success : %d\n",_sockFd);
    }

    void start()
    {
        //1.将标准输入中的数据读取到缓冲区中--缓冲区message
        std::string message;
        while(!_quit)//退出标志不为真时一直进行循环
        {
            std::cout << "Enter#";
            getline(std::cin, message);
            
            //判断退出指令.
            if(strcasecmp(message.c_str(),"quit") == 0)
            {
                _quit = true;
                continue;
            }

            ssize_t s = write(_sockFd, message.c_str(),message.size());
            if(s > 0)//写入成功时,write返回向_sockFd成功写入的除'\0'外的字节数
            {
                message.resize(1024);
                ssize_t n = read(_sockFd,(char*)message.c_str(),1024);
                if(s > 0)
                {
                    message[n]='\0';
                }
                std::cout<<"Server Echo >> " << message << std::endl;
            }
            else if(s == 0)//write没有写入任何内容--即,读端关闭,也是服务端关闭.
            {
                logMessage(NOTICE,"Server shutdown!\n");
                break;
            }
            else
            {
                logMessage(FATAL,"write error : %s", strerror(errno));
                break;
            }
        }

        //关闭套接字
        close(_sockFd);
        logMessage(DEBUG,"Client quit safe!\n");
    }

private:
    uint16_t _servicePort;
    std::string _serviceIp;
    int _sockFd;
    bool _quit;
};



void useManual()
{
    std::cout << "command port ip" << std::endl;
}

int main(int argc, char* args[])
{
    if(argc != 3)
    {
        useManual(); 
        exit(1);
    }
    
    uint16_t servicePort = atoi(args[1]);
    std::string serviceIp = args[2];
    tcpClient client(servicePort,serviceIp);
    client.init();
    client.start();
    return 0;
}
