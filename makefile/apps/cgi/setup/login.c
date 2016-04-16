#include <cgi/cgi.h>
#include "login.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


int set_rem(char *input, char *errmsg)
{
    //ALERT(input);
    return 0;
}

int get_rem(char *input, char *errmsg)
{
    strcpy(input, "1");
    return 0;
}


int set_username(char *input, char *errmsg)
{
    if (strcmp(input, "admin")) {
        ALERT("Username Error!");
        HTML_GOTO("index.htm");
        return -1;
    }
    return 0;
}

int get_username(char *input, char *errmsg)
{
    return 0;
}

int set_password(char *input, char *errmsg)
{
    if (strcmp(input, "admin")) {
        ALERT("Password Error!");
        HTML_GOTO("index.htm");
        return -1;
    }
    return 0;
}

int action_login(char *outbuf, char *errmsg, cgi_func_tab_t *func_tab)
{
     
    return 0;
}

