#include"HttpServer.hpp"

void useManual()
{
    std::cout << "./HttpServer port [ip]" << std::endl;
    std::cout << "example: ./HttpServer 8080" << std::endl;
}

int main(int argc, char* args[])
{
    if(argc != 2 && argc != 3)
    {
        useManual();
    }
    
    uint16_t port = atoi(args[1]);
    std::string ip = args[2] == nullptr ? "" : args[2];

    HttpServer server(ip, port);
    server.init();
    server.loop();
    return 0;
}