#include"util.hpp"
#include"Log.hpp"
#include"protocol.hpp"
#include"threadPool.hpp"
#include"task.hpp"

#include<string>
#include<cstring>

#include<signal.h>

#define SERVER
#define BYZERO -1

    //Core service
    void calculator(Request &req ,Response &res)
    {
        int x = req.getLhs();
        int y = req.getRhs();
        char op = req.getOp();
        
        int result = 0;
        int exitCode = 0;
        switch(op)
        {
            case '+':
            { result = x + y; } break;
            case '-':
            { result = x - y; } break;
            case '*':
            { result = x * y; } break;
            case '/':
            {
                //除数不能为0
                if(y == 0)
                { exitCode = BYZERO; }
                else
                { result = x / y; }
            } break;
            case '%':
            {
                //除数不能为0
                if(y == 0)
                { exitCode = BYZERO; }
                else
                { result = x % y; }
            } break;
            default: break;
        }

        res.setExitCode(exitCode);
        res.setResult(result);
    }

    //服务端提供的网络服务
    void netCal(int sock, std::string clientIp, uint16_t clientPort)
    {
        assert(sock >= 0);
        assert(!clientIp.empty());
        assert(clientPort >= 1024);

        std::string inbuffer;
        while(true)
        {
            //1.服务创建
            //1.1.接收客户端请求
            Request req;
            char message[256];
            int n = read(sock, message, sizeof(message)-1);
            if(n == 0)
            {
                logMessage(WARNING,"Client[%s : %ld] shutdown | socket:%d\n", \
                            clientIp.c_str(), clientPort, sock);
                break;
            }
            else if(n == -1)
            {
                logMessage(WARNING,"Client[%s : %ld] error:%s | socket:%d\n", \
                            clientIp.c_str(), clientPort,\
                            strerror(errno), sock);
                break;
            }
            //走到这里,读取成功.
            message[n] = '\0';
            inbuffer += message;
            std::cout << "inbuffer:" << inbuffer << std::endl;
            //2.解析报文
            //2.1.检查报文完整性
            size_t packageLen = 0;
            std::string package = decode(inbuffer, &packageLen);
            if(packageLen == 0) continue;//尚未获取到完整的报文结构
            std::cout << "package:" << package << " | packageLen:" << packageLen << std::endl;
            //2.2已经获取到了有效载荷的完整协议报文,反序列化
            if(req.deserialize(package)) //反序列化失败---报文格式错误
            {
                req.deBug();
                
                //3.获取请求,对请求进行处理---即返回一个response
                Response res;
                calculator(req, res);
                res.deBug();

                //4.序列化.
                std::string outBuffer;
                res.serialize(outBuffer);
                
                //4.添加报头
                std::string out = encode(outBuffer, outBuffer.size());
                std::cout << out.size() << std::endl;

                //5.传输
                int s = write(sock, out.c_str(), out.size());
                if(s > 0)
                {   
                    logMessage(DEBUG, "Client[%s:%d] request has been answered | socket:%d\n",\
                              clientIp.c_str(), clientPort, sock);  
                }
                else if(s == 0)
                {
                    logMessage(FATAL,"Client[%s:%d] shutdown | socket:%d\n",\
                              clientIp.c_str(), clientPort, sock);
                    break;
                }
                else
                {
                    logMessage(FATAL,"Client[%s:%d] receive error:%s | socket:%d\n",\
                              clientIp.c_str(), clientPort, strerror(errno), sock);
                    break;
                }
            }
        }

        //关闭已经使用完毕的套接字
        close(sock);
    }


class TcpServer
{
public:
    TcpServer(std::string ip, uint16_t port) : 
    ownIp(ip), 
    ownPort(port), 
    sockFd(-1),
    quit(false),
    tp(nullptr)
    {}
    ~TcpServer()
    {}

public:
    void init()
    {
        //1.创建套接字
        sockFd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockFd == -1)
        {
            logMessage(FATAL, "socket:%d | socket error:%s\n", sockFd, strerror(errno));
            exit(1);
        }

        //2.绑定套接字信息
        //2.1.填充信息结构体-struct sockaddr_in
        struct sockaddr_in local;
        local.sin_family = PF_INET;
        local.sin_addr.s_addr = ownIp.empty() ? INADDR_ANY : inet_addr(ownIp.c_str());
        local.sin_port = htons(ownPort);
        //2.2.绑定信息到内核,等待被跨网络访问
        if(bind(sockFd, (const sockaddr*)&local, sizeof(local)) != 0)
        {
            logMessage(FATAL, "socket:%d | bind error:%s\n",sockFd, strerror(errno));
            exit(2);
        }

        //3.创建监听套接字
        if(listen(sockFd,5) != 0)
        {
            logMessage(FATAL, "socket:%d | listen error:%s\n",sockFd, strerror(errno));
            exit(3);
        }

        //4.初始化线程池
        tp = threadPool<Task>::getInstance();
        tp->start();
    }   

    void loop()
    {
        while(!quit)
        {
            //用来接收远端申请连接的主机的套接字信息
            struct sockaddr_in peer;        //输出型参数
            socklen_t len = sizeof(peer);   //输入输出型参数
            memset(&peer, 0, len);
            //4.使用accpet阻塞等待获取未决连接队列中的首位
            int serviceSock = accept(sockFd, (struct sockaddr*)&peer,&len);
            if(serviceSock == -1)
            {
                logMessage(FATAL, "socket:%d | listen error:%s\n",sockFd, strerror(errno));
                continue;
            }
            //4.1.获取对端的套接字信息
            std::string peerIp = inet_ntoa(peer.sin_addr);
            uint16_t peerPort = ntohs(peer.sin_port);
            //4.2将服务push到线程池中开始执行.
            Task t(serviceSock, peerIp, peerPort, netCal);    
            tp->push(t);       
        }
    } 

    bool quitServer()
    {
        quit = true;
        return true;
    }

private:
    int sockFd;
    std::string ownIp;
    uint16_t ownPort;
    bool quit;
    threadPool<Task> *tp; //单例线程池
};

void useManual()
{
    std::cout << "Exec fomat: ./command port [ip]" << std::endl;
    std::cout << "example: ./tcpServer 8081 127.0.0.1" << std::endl; 
}
TcpServer *svrp = nullptr;

void sigHandler(int signal)
{
    if(signal == 3 && svrp != nullptr)
    {
        //退出标志置位.
        svrp->quitServer();
        logMessage(DEBUG,"Safe quit\n");
    }
}

int main(int argc, char* args[])
{
    if(argc != 2 && argc!= 3)
    {
        useManual();
        exit(1);
    }

    signal(3,sigHandler);
    
    uint16_t port = atoi(args[1]);
    std::string ip = args[2] == nullptr ? "" : ip;
    svrp = new TcpServer(ip,port);
    svrp->init();
    svrp->loop();
    return 0;
}