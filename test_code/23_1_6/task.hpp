#pragma once
#include<iostream>
#include<functional>
#include "Log.hpp"

class Task
{
public:
    using callback_t = std::function<void(int, std::string, uint16_t)>;
    //等价于:
    //typedef std::function<void (*)(int,std::string,uint16_t)> callback_t; 
    Task(int sock, std::string ip, uint16_t port, callback_t func) :
    _sock(sock),
    _ip(ip),
    _port(port),
    _func(func) 
    {}
    ~Task()
    {}

public:

    void operator()()
    {
        logMessage(DEBUG,"线程ID[%p]处理[%s:%d]的请求 -- start...\n", pthread_self(), _ip.c_str(), _port);
        _func(_sock, _ip, _port);
        logMessage(DEBUG,"线程ID[%p]处理[%s:%d]的请求 -- done...\n", pthread_self(), _ip.c_str(), _port);
    }

private:
    int _sock;          //为用户提供IO服务的套接字
    std::string _ip;    //客户端的IP
    uint16_t _port;     //客户端的端口号
    callback_t _func;   //回调方法.
};