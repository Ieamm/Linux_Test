#include<string>
#include<cstring>
#include<iostream>
#include<cassert>
#include<sstream>

#include<jsoncpp/json/json.h>

#define JSON
#define CRLF "\r\n"
#define CRLF_LEN strlen(CRLF)
#define SPACE " "
#define SPACE_LEN strlen(SPACE)

const char OPS[] = {'+','-','*','/','%'};

//添加报文.
std::string encode(std::string & out, size_t len)
{
    assert(len > 0);

    std::string message = std::to_string(len);
    message += CRLF;
    message += out;
    message += CRLF;

    return message;
}

//解析报文.
std::string decode(std::string& in, size_t *len)
{
    assert(len != nullptr);
    //1.因为字节流的写入和读取都是不定长,且读写端同时工作的话属于"写多少读多少"的状态,不能保证具体的传输内容.\
    因此对是否接收到了完整的报头(有效载荷长度)进行监测.
    *len = 0;
    size_t headerPos = in.find(CRLF);
    if(headerPos == std::string::npos)//表示没有查找到报头的分隔符,即完整的报头没有被传输过来.
    {
        return "";
    }
    //1.1.走到这里说明此时已经接收到了完整报头,获取字符串形式报头.
    std::string inHeader = in.substr(0,headerPos);
    
    //2.使用字符串流转换字符串类型为整形.
    size_t header;//用来确认当前是否读取到所有有效载荷.
    std::stringstream ss;
    ss << inHeader;
    ss >> header;

    //3.确认当前读取到的有效载荷长度是否是全部长度
    std::cout << "报文全长:" << in.size() << std::endl;
    int surplus = in.size() - headerPos - CRLF_LEN * 2;//得到当前有效载荷长度.
    if(surplus < header)//当前有效载荷长度<报头有效载荷长度=未读取到全部报文.
    {
        std::cout << "未获取到全部报头格式,等待下一次读取结果." << std::endl;
        return "";
    } 
    
    //4.到这里证明拥有完整的报文结构
    *len = header;
    std::string package = in.substr(headerPos+CRLF_LEN,*len);
    
    //5.又是由于字节流的写入和读取机制的原因---对字节流写入时,读取端没有立刻进行读取,则下一次读取会直接将所有内容读取.\
    也就导致可能出现:客户端连续两次操作所发送的数据被归纳到一次读取中.\
    因此需要将当前已知的完整报文结构从in(读取缓冲区)中抹除
    size_t removeLen = package.size() + headerPos + (CRLF_LEN * 2);
    in.erase(0, removeLen);
    
    return package;
}

//请求
class Request
{
public:
    Request()
    {}
    ~Request()
    {}
public:
    //序列化--将结构化数据转换为字符串
    void serialize(std::string &out)
    {
#ifndef JSON
        //统一格式:左操作数+空格+操作符+空格+右操作数
        out = std::to_string(lhs);
        out += SPACE;
        out += op;
        out += SPACE;
        out += std::to_string(rhs);
#else
        //创建万能对象root
        Json::Value root;
        //建立结构体数据字段在万能对象中的映射
        root["lhs"] = lhs;
        root["rhs"] = rhs;
        root["op"] = op;

        //创建FastWriter类,进行json串向string的转换
        Json::FastWriter fw;
        //将json格式的数据字段写入到string对象中.
        out = fw.write(root);
        std::cout << "\n" << out << std::endl;
#endif
    }

    //反序列化--将字符串转换为结构化数据
    bool deserialize(std::string &in)
    {
#ifndef JSON
        size_t oneSpacePos = in.find(SPACE);
        if(oneSpacePos == std::string::npos) return false;
        size_t twoSpacePos = in.rfind(SPACE);
        if(twoSpacePos == std::string::npos) return false;
        
        //111 + 241
        int left = std::stoi(in.substr(0, oneSpacePos));
        std::string op = in.substr(oneSpacePos + SPACE_LEN, twoSpacePos - (oneSpacePos - SPACE_LEN));
        int Right = std::stoi(in.substr());
        
        setLhs(left);
        setRhs(Right);
        setOp(op[0]);
    
        return true;
#else
        //json
        Json::Value root;
        Json::Reader rd;
        //将json格式串in的内容读到万能对象root中
        rd.parse(in,root);
        //使用已经在万能对象中建立了映射的键,访问字段数据,并将其转换为指定的数据类型.
        lhs = root["lhs"].asInt();
        rhs = root["rhs"].asInt();
        op = root["op"].asInt();
        return true;
#endif
    }

    void deBug()
    {
        std::cout << "-----------------------------" << std::endl;
        std::cout << "lhs:" << lhs << std::endl;
        std::cout << "op:" << op << std::endl;
        std::cout << "rhs:" << rhs << std::endl;
        std::cout << "-----------------------------" << std::endl;
    }

public:
    int getLhs()
    { return lhs; }
    int getRhs()
    { return rhs; }
    char getOp()
    { return op; }
    void setLhs(int parLhs)
    { lhs = parLhs; }
    void setRhs(int parRhs)
    { rhs = parRhs; }
    void setOp(int parOp)
    { op = parOp; }

private:
    int lhs;
    int rhs;
    char op;
};

//响应
class Response
{
public:
    Response()
    {}
    ~Response()
    {}

public:
    void serialize(std::string &out)
    {
#ifndef JSON
        out = std::to_string(exitCode);
        out += SPACE;
        out += std::to_string(result);
#else
        //json
        Json::Value root;
        root["exitCode"] = exitCode;
        root["result"] = result;

        Json::StyledWriter sw;
        out = sw.write(root);
        std::cout << "\n"<< out << std::endl;
#endif
    }

    bool deserialize(std::string &in)
    {
#ifndef JSON
        size_t spacePos = in.find(SPACE);
        if(spacePos == std::string::npos) return false;
        
        int exitValue = stoi(in.substr(0,spacePos));
        int ret = stoi(in.substr(spacePos + SPACE_LEN));

        setExitCode(exitValue);
        setResult(ret);
        return true;
#else
        //json
        Json::Value root;
        Json::Reader rd;
        rd.parse(in,root);
        
        exitCode = root["exitCode"].asInt();
        result = root["result"].asInt();
        return true;
#endif 
    }

    void deBug()
    {
        std::cout << "-----------------------------" << std::endl;
        std::cout << "exitCode:" << exitCode << std::endl;
        std::cout << "result:" << result << std::endl;
        std::cout << "-----------------------------" << std::endl;
    }

public:
    int getExitCode()
    { return exitCode; }
    int getResult()
    { return result; }
    void setExitCode(int parExitCode)
    {  exitCode = parExitCode; }
    void setResult(int parResult)
    { result = parResult; }

private:
    int exitCode;
    int result;
};



bool makeRequest(char *in, Request &req)
{
    //只考虑1+1 10*32 187+82等格式
    char strTemp[1024];
    //将格式化输入内容写入字符串.
    snprintf(strTemp, sizeof(strTemp), "%s", in);
    char* strLhs = strtok(strTemp, OPS);
    if(!strLhs)  return false;
    char* strRhs = strtok(NULL, OPS);
    if(!strRhs)  return false;
    char operate = in[strlen(strLhs)];//187+82 : strlen(187)==3 -> in[3]==(+)
     
    req.setLhs(atoi(strLhs));
    req.setRhs(atoi(strRhs));
    req.setOp(operate);
    return true;
}
