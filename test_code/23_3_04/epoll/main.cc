#include "EpollServer.hpp"
#include "log.hpp"
using namespace std;

const int NUM = 1042;
void useage()
{ 
    cout << "./Server Port [ip]"
         << "example:"
         << "./Server 8080 127.0.0.1" << endl;
}

int myFunc(int sock)
{
    //TODO -- BUG
    char* buffer[NUM];
    int n = read(sock, buffer, NUM);
    if(n > 0)
    {
        buffer[n] = 0;
        logMessage(DEBUG, "cliten[%d]# %s", sock, buffer);        
    }

    return n;
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
    EpollServer epollEvent(port,myFunc);
    epollEvent.initEpoll();
    epollEvent.run();
    return 0;
}