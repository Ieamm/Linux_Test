#include<string>
#include<iostream>

#define SEPARATOR "_"
#define SEPARATOR_LEN strlen(SEPARATOR)
#define CRLF "\r\n"
#define CRLF_LEN strlen(CRLF_LEN)

struct Request
{
    int lhs;
    char op;
    int rhs;
public:
    int operator() ()
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
                    return -1;
                }
                result = lhs / rhs; break;
            }
        }

        return result;
    }
};

struct Response
{
    int Code;
    double result;
};

struct Protocol
{
    std::string DeSrialization(std::string &in)
    {
        
    }

    void Decode(std::string &in, int len)
    {

    }
};