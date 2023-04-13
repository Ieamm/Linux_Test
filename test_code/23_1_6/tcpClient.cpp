#include"Log.hpp"
#include"util.hpp"
#include"protocol.hpp"

#include<string>
#include<iostream>
#include<cstring>

#define SAFE_SHUTDOWN INT32_MIN

class TcpClient
{
public:
    TcpClient(const std::string ip, const uint16_t port):
    serverIp(ip), 
    serverPort(port), 
    sockFd(-1), 
    quit(false)
    {}
    ~TcpClient()
    {
        if(sockFd != -1)
        {
            close(sockFd);
        }
    }
    
public:
    void init()
    {
        //1.创建套接字
        sockFd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockFd == -1)
        {
            logMessage(FATAL,"socket:%d | create error:%s\n", sockFd, strerror(errno));
            exit(2);
        }
        //无需bind
        //2.申请与远端服务器建立连接(connect)
        //2.1.填充客户端结构体
        struct sockaddr_in serviceSockaddr;
        size_t len = sizeof(serviceSockaddr);
        memset(&serviceSockaddr, 0, len);
        serviceSockaddr.sin_family = AF_INET;
        serviceSockaddr.sin_port = htons(serverPort);
        if(serverIp == "")
        { serviceSockaddr.sin_addr.s_addr = INADDR_ANY; }
        else
        {
            if(inet_aton(serverIp.c_str(), &serviceSockaddr.sin_addr) == 0)
            {
                logMessage(FATAL, "Invaild ip format:%s | socket:%d\n", serverIp.c_str(), sockFd);
                exit(3);
            }
        }
        //2.2申请链接
        if(connect(sockFd, (const sockaddr*)&serviceSockaddr, len) == -1)
        {
            logMessage(FATAL,"connect error:%s | socket:%d\n", serverIp.c_str(), sockFd);
            exit(4);
        }
    }

    void start()
    {
        while(!quit)
        {
            int s = requestToServer();
            if (s > 0)
            {   getResponse();  }
            else if (s == SAFE_SHUTDOWN) // 表示安全退出
            {
                logMessage(DEBUG, "Client safe shutdown | socket:%d\n",sockFd);    
                break;
            }
            else // requestToServer的返回值等于0代表服务端关闭,小于0代表的是写入错误
            {   break;    }
        }
    }

private:
    int requestToServer()
    {
        // 1.读取输入
        char data[1024];
        std::cout << "Place Enter#";
        fgets(data, sizeof(data), stdin); // 从标准输入中读取一行数据到outBuffer

        if (strcasecmp(data, "quit") == 0)
        {
            quit = true;
            return SAFE_SHUTDOWN;
        }
        // 2.申请远端服务
        Request req;
        // 2.1.填充申请.
        makeRequest(data, req);
        req.deBug();
        // 2.2.将申请服务的结构化数据序列化
        std::string outBuffer;
        req.serialize(outBuffer);
        // 2.3.为序列化的数据添加报头和协议格式
        std::string out = encode(outBuffer, outBuffer.size());
        // 2.4.将转换好的协议数据发送给对端的服务器
        return write(sockFd, (void *)out.c_str(), out.size());
    }

    void getResponse()
    {
        // 3.接收对端服务器返回的结果
        // 3.1.接收服务端传回的协议数据
        std::string inBuffer;
        while (true)
        {
            // 因为字节流传输中写入和读取的不确定性,因此需要对每次读取到的数据进行检查.
            char dataSegment[1024];
            int n = read(sockFd, dataSegment, 1024);
            if(n > 0)
            {
                // 3.2.将协议数据进行解包
                inBuffer += dataSegment;
                size_t len = 0;
                // 检查数据完整性--decode
                std::string package = decode(inBuffer, &len);
                if (package == "")
                {
                    continue;
                }
                Response res;
                // 反序列化
                if(res.deserialize(package))
                {
                    std::cout << "exitCode:" << res.getExitCode()
                          << " | result:" << res.getResult() << std::endl;
                }
                else
                {
                    std::cout << "deserialize error" << std::endl;
                }

                break;
            }
            else if(n == 0)
            {
                logMessage(FATAL,"Server shutdown | socket %d\n",sockFd);
                exit(1);
            }
            else
            {
                logMessage(FATAL,"Data receive error:%s | socket %d\n", strerror(errno), sockFd);
                exit(2);  
            }
        }
    }

private:
    std::string serverIp;
    uint16_t serverPort;
    int sockFd;
    bool quit;
};

void useManual()
{
    std::cout << "Exec fomat: ./command port [ip]" << std::endl;
    std::cout << "example: ./tcpClient 8081 127.0.0.1" << std::endl; 
}

int main(int argc, char* args[])
{
    if(argc != 2 && argc!= 3)
    {
        useManual();
        exit(1);
    }
    
    uint16_t port = atoi(args[1]);
    std::string ip = args[2] == nullptr ? "" : args[2];
    TcpClient client(ip,port);
    client.init();
    client.start();
    return 0;
}