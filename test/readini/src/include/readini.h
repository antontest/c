#ifndef __READINI_H__
#define __READINI_H__

#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

//从自己ini配置文件中读取信息，lpReturnedString为返回值，nsize为返回值缓冲区大小
int GetPrivateProfileString(const char *lpAppName,const char *lpKeyName,const char *lpDefault,char * lpReturnedString,int nsize,const char *lpFileName);

#endif
