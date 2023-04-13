#include<stdio.h>
#include<assert.h>
#include<string.h>
int main()
{
    FILE* pf = fopen("./bite","w+");//以读写方式打开文件,不存在就创建
    assert(pf!=NULL);

    const char* msg = "linux so easy!";
    fwrite(msg,sizeof(char),strlen(msg),pf);//写入msg

    char buf[20];
    memset(buf,'\0',sizeof(buf)/sizeof(char));//初始化buf
    fseek(pf,0,SEEK_SET);//文件流指针归位
    fread(buf,sizeof(buf),sizeof(char),pf);//读取

    fprintf(stdout,"%s\n",buf);//向stdout打印
    fclose(pf);    //关闭文件
    return 0;
}