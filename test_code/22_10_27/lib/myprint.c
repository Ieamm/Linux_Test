#include"myprint.h"

void Print(const char* msg)
{
  printf("%s---%lld\n",msg,(long long)time(NULL));
}
