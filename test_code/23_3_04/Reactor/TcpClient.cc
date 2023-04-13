#include<iostream>
#include "Sock.hpp"
#include "protocol.hpp"
#include"log.hpp"
using namespace std;

class TcpClient
{
public:
    TcpClient(std::string ip, uint16_t port):sock_(-1), ip_(ip), port_(port)
    {}

    ~TcpClient()
    {}

    void init()
    {
        sock_ = Sock::Socket();
        Sock::Connect(sock_, ip_, port_);
    }

    void Run()
    {
        while(true)
        {
            Request req;
            std::cout << "lhs: ";
            std::cin >> req.lhs;
            std::cout << "op: ";
            std::cin >> req.op;
            std::cout << "rhs: ";
            std::cin >> req.rhs;

            std::string outbuffer;
            req.Serialization(outbuffer);
            Protocol::Package(outbuffer);

            int n = send(sock_, outbuffer.c_str(), outbuffer.length(), 0);
            if(n < 0)
            {
                logMessage(FATAL, "send error sock[%d]\n", sock_);
                break;
            }
            else if(n == 0)
            {
                logMessage(NOTICE, "Peer shutdown sock[%d]\n", sock_);   
                break;
            }
            else
            {;}
        }
    }
private:
    int sock_;
    std::string ip_;
    uint16_t port_;
};

int main()
{
   
    return 0;
}