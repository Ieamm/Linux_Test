#pragma once

#include <iostream>
#include <ctime>

std::ostream &Log()
{
    std::cout << "Fot Debug |" << " timestamp: " << (uint64_t)time(nullptr) << " | ";
    return std::cout;
}