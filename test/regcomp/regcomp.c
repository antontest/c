
    #include "time.h"  
    #include "stdio.h"  
    int main(void)  
    {  
        time_t lt      = time(NULL);  
        struct tm* ptr = localtime(&lt);  
        char szBuffer[512] = {0};  
        const char* pFormat = "                        ENodeBApp       Info    %Y-%m-%d %H:%M:%S.861687      0       61849   No Msg3 received and PRACH count reached to Max! PRACH=[125].";  
        strftime(szBuffer, 512, pFormat, ptr);  
        printf("%s\n", szBuffer);
        
        const char *format = "                        ENodeBApp       Info    %F %T";
        struct tm tmTemp = {0};  
        char tmBuffer[512] = {0};  
        strptime("                        ENodeBApp       Info    2015-05-10 15:26:34.861687      0    61849   No Msg3 received and PRACH count reached to Max! PRACH=[125]", format, &tmTemp);
        strftime(tmBuffer, 512, "%Y-%m-%d %H:%M:%S", &tmTemp);  
        printf("%s\n", tmBuffer);  
        return 0;  
    }  
