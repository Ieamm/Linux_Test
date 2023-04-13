#pragma once
#include<iostream>

class task
{
public:
    task(int lhs=0, int rhs=0, char operate='0') : _lhs(lhs),_rhs(rhs),_operate(operate)
    {
        ;
    }
    ~task()
    {}

    int operator() ()
    {
        return run();
    }

    int run()
    {
        int result = 0;
        switch(_operate)
        {
            case '+':
            {
                result = _lhs + _rhs;
                break;
            }
             case '-':
            {
                result = _lhs - _rhs;
                break;
            }
             case '*':
            {
                result = _lhs * _rhs;
                break;
            }
             case '/'://除和取模都需要注意除零错误的发生
            {
                if(_rhs == 0)
                {
                    std::cout << "div zero error" << std::endl;
                    break;
                }
                result = _lhs / _rhs;
                break;
            }
             case '%':
            {
                if(_rhs == 0)
                {
                    std::cout << "mod zero error" << std::endl;
                    break;
                }
                result = _lhs % _rhs;
                break;
            }
            default:
            {
                std::cout << "operator error" << std::endl;
                break;
            }
        }

        return result;
    }

    void get(int *lhs, int *rhs, char *operate)
    {
        *lhs = _lhs;
        *rhs = _rhs;
        *operate = _operate;
    }

private:
    int _lhs;
    int _rhs;
    char _operate;
};