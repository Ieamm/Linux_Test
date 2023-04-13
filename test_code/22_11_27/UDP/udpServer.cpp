#include"util.h"
#include"log.hpp"

#include<iostream>
#include<cstdint>
#include<unordered_map>

#include<sstream>
#include<string>
#include<cstring>

using namespace std;

class UdpServer
{
public:
    UdpServer(uint16_t port, const string ip) : 
    _port(port),
    _ip(ip),
    _sockFd(-1)
    {}
    ~UdpServer()
    {}

public:
    void init()
    {
        //1.创建socket套接字--socket
        _sockFd = socket(AF_INET,SOCK_DGRAM,0);
        if(_sockFd == -1)
        {
            logMessage(FATAL,"socket error:%s %d\n",strerror(errno),_sockFd);
            exit(2);
        }
        logMessage(DEBUG,"socket success:%d\n",_sockFd);
        
        //2.绑定网络信息--bind
        //2.1填充基本信息到网络协议信息结构体sockaddr_in
        sockaddr_in sa_in;
        memset(&sa_in,0,sizeof(sa_in));
        //协议家族
        sa_in.sin_family = AF_INET;
        //端口号
        sa_in.sin_port = htons(_port);
        //IP地址--因为需要向远端机器发送,因此需要将本机序列转换为网络序列
        //inet_addr会自动将转换出来的in_addr_t返回值转换为网络序列
        //并且因为ip地址传入时是点分十进制的字符串,在传输中为了省空间使用inet_addr接口转换为四字节大小.
        sa_in.sin_addr.s_addr = _ip.empty() ? htonl(INADDR_ANY) : inet_addr(_ip.c_str());
        //2.2bind
        if(bind(_sockFd,(const sockaddr*)&sa_in,sizeof(sa_in)) == -1)
        {
            logMessage(FATAL,"bind error:%s:%d\n",strerror(errno),_sockFd);
            exit(3);
        }
        logMessage(DEBUG,"bind success:%d\n", _sockFd);
    }

    void start()
    {
        //服务器作为持续提供服务的进程,必然要被设置为死循环
        char net_inbuffer[1024];//网络数据的读取缓冲区
        while(true)
        {
            //调用recvfrom来接收网络信息
            //1.接收远端发来的信息
            //输出型参数--接受对端的网络信息
            struct sockaddr_in peer_in;   
            //输入输出型参数--传输该结构体大小,并接收传来的对端网络信息结构体大小
            socklen_t len = sizeof(peer_in);
            bzero((void*)&peer_in,len);
            ssize_t n = recvfrom(_sockFd, net_inbuffer, sizeof(net_inbuffer) - 1, 0,
                                 (struct sockaddr *)&peer_in, &len);
            if(n > 0)
            {
                //因为网络数据到了语言内部是没有'\0'的,因此需要加上'\0'作为终止符
                //并且recvfrom的返回值是接收的数据的字节数,-1后可以作为下标被用为'\0'的存储位置.
                net_inbuffer[n] = '\0';
            }
            else if(n == 0) //所有的对端都已经关闭了
            {
                logMessage(DEBUG,"all peer perform orderly shutdown\n");
                continue;
            }
            else //recvfrom函数运行错误
            {
                logMessage(WARNING,"recvfrom erro : %s %d\n",strerror(errno),_sockFd);
                continue;
            }
            string peer_ip = inet_ntoa(peer_in.sin_addr);
            uint16_t peer_port = ntohs(peer_in.sin_addr.s_addr);
            
            //2.打印信息
            logMessage(NOTICE,"[%s:%d]#%s\n",peer_ip.c_str(),peer_port,net_inbuffer);

            //3.录入联网用户--传入网络序列对象peer_in
            checkOnlineUsers(peer_ip,peer_port,peer_in);

            //4.将信息回执给所有远端.
            messageRoute(peer_ip,peer_port,net_inbuffer);
        }
        logMessage(DEBUG,"Server shutdown!\n");
    }

    void checkOnlineUsers(const string& ip,const uint16_t port,struct sockaddr_in &peer)
    {
        string key = ip;
        key += ":";
        key += to_string(port);
        auto iter = _users.find(key);
        if(iter == _users.end())//若该用户未存在,则添加
        {
            _users.insert(make_pair(key,peer));
        }
        else//若该用户存在,则不添加.
        {
            ;
        }
    }

    void messageRoute(const string &ip, uint16_t port, const char *info)
    {
        string message = "[";
        message += ip;
        message += ":";
        message += to_string(_port);
        message += "]#";
        message += info;

        for(auto user : _users)
        {
            //这里的user.second所取出的也是网络序列对象.
            if(sendto(_sockFd, message.c_str(), message.size(), 0,
                   (const struct sockaddr *)&user.second, (socklen_t)sizeof(user.second)) == -1)
            {
                logMessage(WARNING,"sendto error : %s %d\n",strerror(errno),_sockFd);
            }
        }
    }

private:
    const uint16_t _port;//服务端端口
    const string _ip;//服务器IP
    int _sockFd;//套接字描述符--远端发来的信息存储在套接字文件的缓冲区内?
    unordered_map<string, struct sockaddr_in> _users;
};

void UserManual()
{
    cout << "command port [ip]" << endl;
}

int main(int argc,char* args[])
{
    if(argc != 2 && argc!= 3)
    {
        UserManual();
        exit(1);
    }
    
    //日志文件
    // Log log;
    // log.enbale();

    uint16_t port;
    stringstream ss;
    ss << args[1];
    ss >> port;
    string ip= args[2] == nullptr ? "" : args[2];
    UdpServer svr(port,ip);
    logMessage(DEBUG,"create net server[%s : %d]\n",ip.c_str(),port);
    svr.init();
    svr.start();

    return 0;
}