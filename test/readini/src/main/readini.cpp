#include "../include/readini.h"

int file_fd=-1;
int GetPrivateProfileString(const char *lpAppName,const char *lpKeyName,const char *lpDefault,char * lpReturnedString,int nsize,const char *lpFileName)
{
    //打开配置文件
    int file_fd = open(lpFileName,O_RDONLY);
    if( file_fd < 0 )
    {
        printf("文件打开失败!\n");
        close(file_fd);
        return -1;
    }

    //清空返回值缓冲区
    bzero(lpReturnedString,nsize);
    
    //获取配置文件大小
    int size = lseek(file_fd,0,SEEK_END);
    printf("文件大小为：%dbytes\n",size);

    //信息读取完成，关闭文件
    close(file_fd);
}

int main()
{
    char value[100];
    GetPrivateProfileString("info","name","andong",NULL,0,"ini");
    return 0;
}
