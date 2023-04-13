#include<iostream>
#include<cstring>
#include<cassert>
#include<cstdio>
#include<fstream>

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<wait.h>

#define CRLF "\r\n"
#define SPACE " "
#define SPACE_LEN strlen(SPACE)
#define HOME_PAGE "index.html"
#define ROOT_PATH "wwwroot" //指定的web根目录


std::string GetPath(std::string httpRequest)
{
    size_t requestLine_Pos = httpRequest.find(CRLF);
    if(requestLine_Pos == std::string::npos) 
        return "";
    //GET /wwwroot/index.html http/1.0
    std::string requestLine = httpRequest.substr(0,requestLine_Pos);
    size_t lineSpace_First = requestLine.find(SPACE);
    if(lineSpace_First == std::string::npos)
        return "";
    size_t lineSpace_Second = requestLine.rfind(SPACE);
    if(lineSpace_Second == std::string::npos)
        return "";
    // /wwwroot/index.html
    std::string path = requestLine.substr(lineSpace_First+SPACE_LEN,lineSpace_Second-(lineSpace_First+SPACE_LEN));
    // 当请求资源的文件路径为根目录时,为其返回web主页
    if(path == "/")
        path += HOME_PAGE;
    return path;
}

std::string ReadFile(std::string path)
{
    std::ifstream in(path);
    if(!in.is_open())
        return "404";
    std::string content;
    std::string line;
    while(std::getline(in,line))
        content += line;
    in.close();
    
    return content;
}

void HandlerHttpRequest(int sock)
{
    //接收请求报文
    char inBuffer[10240];
    ssize_t s = read(sock, inBuffer, sizeof(inBuffer)-1);
    assert(s > 0);
    inBuffer[s] = '\0';
    printf("++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("%s\n",inBuffer);

    //1.请求打开的资源文件在哪里?就在请求行中的第二个字段  [/wwwroot/index.html]
    std::string path = GetPath(inBuffer);
    //1.1.设置web根目录为起始目录
    std::string recource = ROOT_PATH;
    recource += path;
    std::cout << recource << std::endl;

    //2.打开资源文件
    std::string html = ReadFile(recource);   
    //2.1.获取文件后缀
    size_t suffixPos = recource.find(".");
    std::string suffix = recource.substr(suffixPos+1);

    //响应报文
    std::string response;

    //body
    //1.状态行
    response = "HTTP/1.0 200 OK\r\n";
    //2.响应报头
    //2.1.正文长度
    if(suffix == ".jpg")    response += "Content-Type: image/jpg\r\n"; /*若请求资源后缀为.jpg...*/
    else                    response += "content-Type: text/html\r\n";
    //2.2.正文长度
    response += ("Content-Length: " + std::to_string(html.size()) + CRLF);//正文长度
    //2.3.设置Cookie
    response += "Set-Cookie: This is my cookie content\r\n";
    //3.响应空行
    response += CRLF;
    //4.响应正文
    response += html;

    // //location
    // response = "HTTP/1.0 301 Temporarily Moved\r\n";//协议/版本 状态码 状态码描述 
    // response += "location: https://www.csdn.net/?spm=1001.2101.3001.4476\r\n";//Location属性
    // response += "\r\n"; //空行

    
    //发送响应
    send(sock, response.c_str(), response.size(), 0);
    close(sock);    
}

class HttpServer
{
public:
    HttpServer(std::string i, uint16_t p):
    sockFd(-1),
    port(p),
    ip(i),
    quit(false)
    {    }
    ~HttpServer()
    {}

public:
    void init()
    {
        //套接字
        sockFd = socket(AF_INET, SOCK_STREAM, 0);
        assert(sockFd != -1);

        //绑定
        struct sockaddr_in own;
        size_t len = sizeof(own);
        memset(&own, 0, len);
        ip, own.sin_family = PF_INET;
        own.sin_port = htons(port); 
        if(ip.empty())
        { own.sin_addr.s_addr = INADDR_ANY; }
        else
        { assert(inet_aton(ip.c_str(), &own.sin_addr) == 0); }
        bind(sockFd, (const struct sockaddr*)&own, len);

        //监听套接字--队列最大连接数5
        assert(listen(sockFd, 5) != -1);
    }
    
    void loop()
    {
        while(!quit)
        {
            struct sockaddr_in peer;  
            socklen_t len = sizeof(peer);
            int serviceSock = accept(sockFd, (sockaddr*)&peer, &len);
            if(quit)
                break;
            assert(serviceSock != -1);

            std::string peerIp = inet_ntoa(peer.sin_addr);
            uint16_t peerPort = ntohs(peer.sin_port);

            pid_t id = fork();
            if(id == 0)//child process
            {
                close(sockFd);
                if(fork() > 0)
                    exit(1);
                //grandchild process -- Taken over by the kernel
                HandlerHttpRequest(serviceSock);
                exit(0);//grandchild become zombie state
            }
            close(serviceSock);
            wait(nullptr);//recycle grandchild process
        }
    }

private:
    int sockFd;
    std::string ip;
    uint16_t port;
    bool quit;
};