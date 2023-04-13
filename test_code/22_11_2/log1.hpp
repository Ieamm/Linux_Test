#pragma once
#include<iostream>
#include<ctime>
//返回一个流对象
//因为流不可拷贝,因此必须返回为&
//且因为流对象需要被进行诸如读取,写入等操作,因此也不适合设计为const
std::ostream& log()
{
    std::cout << "debug log | timestamp : "  << (unsigned long long)time(NULL) << " ";
    return std::cout;
}

