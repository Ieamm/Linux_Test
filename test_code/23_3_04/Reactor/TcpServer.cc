#include "TcpServer.hpp"
#include "log.hpp"
#include "protocol.hpp"
using namespace std;

const int NUM = 1042;
void useage()
{ 
    cout << "./Server Port [ip]"
         << "example:"
         << "./Server 8080 127.0.0.1" << endl;
}

int myFunc(Connection* conn, string& message)
{
    //将解包后的信息反序列化到任务结构中
    Request req;
    req.DeSerialization(message);
    auto ret = req();

    //将任务处理结果写入到响应结构中
    Response rep;
    rep.result = ret.second;
    rep.code = ret.first;
    //将响应结构数据序列化存储到输出缓冲区.
    rep.Serialization(conn->outbuffer_);
    Protocol::Package(conn->outbuffer_);

    //发送数据
    conn->sender_(conn);
    if(!conn->outbuffer_.empty())
    { conn->R_->EnableReadWrite(conn->fd_, true, true); }
    else
    { conn->R_->EnableReadWrite(conn->fd_, true, false); }
}

int main(int argc, char* argv[])
{
    if(argc != 2 && argc != 3)
    {
        exit(3);
        logMessage(DEBUG,"Running Start Error");
        useage();   
    }
    uint16_t port = atoi(argv[1]);
    std::string ip = argv[2] == nullptr ? "" : argv[2];
    
    TcpServer svr(myFunc, atoi(argv[1]));
    svr.Run();
    return 0;
}