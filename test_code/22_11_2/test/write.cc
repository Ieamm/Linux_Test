#include<sys/types.h>                                       
#include<sys/stat.h>
#include<iostream>
#include<unistd.h>
#include<fcntl.h>
using namespace std;

int main()
{
  mkfifo(".fifo",0666);
  int fd = open(".fifo",O_WRONLY);
//  int num = 0;
 // write(fd,&num,sizeof(num));
  return 0;
}
