#include<string>
#include<iostream>
#include<vector>
#include<sstream>

#include <jsoncpp/json/json.h>

#define SEPARATOR "|"
#define SEPARATOR_LEN strlen(SEPARATOR)
#define CRLF "\r\n"
#define CRLF_LEN strlen(CRLF_LEN)

class Request;
class Response;

namespace Protocol
{
    void Package(std::string &outbuffer)
    {
        outbuffer += SEPARATOR;
    }

    void PackageSplit(std::string &inbuffer, std::vector<std::string> &result)
    {
        //14 + 5|18 / 0|14 +
        while(true)
        {
            //根据制定好的协议找到每一段完整数据的分隔符
            size_t sepPos = inbuffer.find(SEPARATOR);
            if(sepPos == std::string::npos) 
                break;
            result.push_back(inbuffer.substr(0, sepPos - 1)); //忽略分隔符|;
            inbuffer.erase(inbuffer.begin(), inbuffer.begin() + sepPos);   
        } 
    }
}


struct Request
{
    int lhs;
    char op;
    int rhs;
public:
    pair<int,int> operator() ()
    {
        int result;
        switch(op)
        {
            case '+':
            { result = lhs + rhs; break;}
            case '-':
            { result = lhs - rhs; break;}
            case '*':
            { result = lhs * rhs; break;}
            case '/':
            {
                if(rhs == 0)
                {
                    std::cout << "除法中右操作数不能为0." << std::endl; 
                    return {1, -1};
                }
                result = lhs / rhs; break;
            }
        }

        return {0, result};
    }

    void Serialization(std::string &out)
    {
        //创建万能对象
        Json::Value root;
        root["lhs", lhs];
        root["op", op];
        root["rhs", rhs];
        
        //将万能对象中存储的字段拼接成Json串
        Json::FastWriter fw;
        out = fw.write(root);
        cout << "Serialization Request:"<< out << endl;
    }

    void DeSerialization(std::string &in)
    {
        //创建万能对象
        Json::Value root;
        //创建读取对象
        Json::Reader jr;
        //将Json串读取到万能象中
        jr.parse(in, root);

        lhs = root["lhs"].asInt();
        op = root["op"].asInt();
        rhs = root["rhs"].asInt();
    }

};

struct Response
{
    int code = 0;
    double result;

public:
    
    void Serialization(std::string &out)
    {
        //创建万能对象
        Json::Value root;
        //将属性字段添加到万能对象内
        root["code"] = code;
        root["result"] = result;
        //创建写对象
        Json::FastWriter fw;
        out = fw.write(root);
        cout << "Serialization Response:"<< out << endl;
    }

    void DeSerialization(std::string &in)
    {
        //创建万能对象
        Json::Value root;
        //创建读对象
        Json::Reader r;
        //将Json串读取到万能对象中
        r.parse(in, root);

        code = root["code"].asInt();
        result = root["result"].asInt();
    }
};