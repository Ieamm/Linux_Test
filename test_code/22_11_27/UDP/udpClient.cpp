#include"util.h"
#include"log.hpp"
#include<string>
#include<strings.h>
#include<sstream>
#include<string.h>
using namespace std;

class UdpClient
{
public:
    UdpClient(uint16_t server_port, string server_ip) : _serverPort(server_port),_serverIp(server_ip),_sockFd(-1)
    {}

    void init()
    {
        //1.创建套接字
        _sockFd = socket(AF_INET,SOCK_DGRAM,0);
        if(_sockFd == -1)
        {
            logMessage(FATAL,"socket error:%s %d\n",strerror(errno),_sockFd);
            exit(2);
        }
        logMessage(DEBUG,"socket success:%d\n",_sockFd);

        //不建议客户端也使用bind进行绑定[ip:port]操作
        //因为在网络通信中, 客户端需要向服务端传递自己的套接字,以便服务端回访和记录.
        //但因为主机内大多数时间都是多端口连接的,并且主机会随机生成端口号,导致通过指定端口号进行网络访问的行为
        //可能由于端口号被占用而无法启动.
    }

    void sendAndReceive()
    {
        //将服务端的网络信息结构体填充,在发送信息的过程中需要以其中的信息为"锚点"找到服务端.
        struct sockaddr_in srv_in;
        memset(&srv_in,0,sizeof(srv_in));
        srv_in.sin_family = AF_INET;                              //服务端的传输协议--IPV4网络协议
        srv_in.sin_port = htons(_serverPort);                     //服务端的端口号--主机转网络.
        //服务端的IP地址--从点分十进制字符串向4字节转换,并且自动第二个参数转换为网络序列
        if(inet_aton(_serverIp.c_str(),&srv_in.sin_addr) == 0)    
        {
            //若ip地址无效,则报错并终止进程.
            logMessage(FATAL,"invaild ip format %d\n",_sockFd);
            exit(4);
        }
        string outBuffer; 

        //创建子线程,使得在等待输入发送的同时可以接收到服务器的信息回执
        pthread_t tid;
        pthread_create(&tid,nullptr,receiveAndPrint,this);

        while(true)
        {
            outBuffer.clear();
            cout << "Enter#";
            getline(cin,outBuffer); 
            //不区分大小写的对两个字符串进行相等运算
            if(strcasecmp(outBuffer.c_str(),"quit") == 0)
            {
                logMessage(DEBUG,"exit client %d\n",_sockFd);
                break;
            }

            //当我们首次调用sendto函数时,client端会自动绑定:[由系统决定使用的ip : 随机生成的端口号].
            if(sendto(_sockFd, outBuffer.c_str(), outBuffer.size(), 0,
            (const struct sockaddr *)&(srv_in), sizeof(srv_in)) == -1)
            {
                logMessage(WARNING,"sendto error : %s %d\n",strerror(errno),_sockFd);
            }
        }

        close(_sockFd);
    }

    static void* receiveAndPrint(void* arg)
    {   
        //即使是static成员函数,也是类内成员.可以使用private成员.
        //但因为其没有this指针,所以就需要显式的传入一个类对象来调用private;
        const UdpClient *Client_p = (const UdpClient*)arg;
        
        struct sockaddr_in src_in;
        socklen_t len = sizeof(src_in);
        memset(&src_in,0,len);
        
        char inBuffer[1024];
        while(true)
        {
            int n = recvfrom(Client_p->_sockFd, inBuffer,sizeof(inBuffer)-1,0,\
                            (struct sockaddr*)&src_in, &len);
            if(n > 0)
            {
                inBuffer[n] = '\0';
                //打印客户端的回执信息
                cout << inBuffer << endl;
                cout << "server echo : " << inBuffer << endl;
            }
            else if(n == 0)
            {
                logMessage(NOTICE,"Server shutdown!\n");
                exit(2);
            }
            else
            {
                logMessage(WARNING,"recvfrom error %s %d\n",strerror(errno),Client_p->_sockFd);
                exit(3);
            }
        }
        return nullptr;
    }

    ~UdpClient()
    {}
private:
    //那客户端不需要[ip:port]套接字吗?当然需要!但那是由操作系统为我们提供,在sendto()第一次调用时自动为我们绑定的. 
    uint16_t _serverPort; //存储服务端的端口号
    string _serverIp;     //存储服务端的ip地址
    int _sockFd;
};

void UserManual()
{
    cout << "command port [ip]" << endl;
}

int main(int argc, char* args[])
{
    if(argc != 2 && argc != 3)
    {
        UserManual();
        exit(1);
    }
    
    stringstream ss;
    uint16_t server_port;
    ss << string(args[1]);
    ss >> server_port;
    string server_ip = args[2] == nullptr ? "" : args[2];
    
    UdpClient Client(server_port, server_ip);
    
    Client.init();
    Client.sendAndReceive();
    return 0;
}