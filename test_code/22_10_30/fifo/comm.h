#include<cstdio>
#include<iostream>
#include<sys/types.h>
#include<sys/stat.h>
#include<cstring>
#include<fcntl.h>
#include<cstdlib>
#include<unistd.h>
#include<errno.h>
using namespace std;

//文件相对路径
#define IPC_PATH "./fifo"