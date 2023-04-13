#include"mymath.h"
int valOfTotal(int val, int target)
{
  int result = 0;
  for(int i = val; i <= target; ++i)
    result += i;

  return result;
}
