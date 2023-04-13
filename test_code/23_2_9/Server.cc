#include "Server.hpp"

int main(int argc, char* args[])
{
    if(argc != 2 && argc != 3)
        exit(3);
    
    std::string ip = args[2] == nullptr ? "" : args[2];
    uint16_t port = atoi(args[1]);
    Server svr(ip,port);
    svr.init();
    svr.loop();
    return 0;
}