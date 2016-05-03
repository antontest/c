#include <cgi/cgi.h>
#include "login.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sql3/sql3.h>

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

static char user_name[20];
static int user_right = 0;
static int sql_username(void *arg, int cnt, char **value, char **name)
{
    user_right = 1;
    strcpy(user_name, value[0]);
    return 0;
}

#define DATABASE_PATH "/home/anton/web/html/db/user.db"
int set_username(char *input, char *errmsg)
{
    char sqlstr[128] = {0};
    sqlite_t *sql = sqlite_create();

    if (access(DATABASE_PATH, R_OK) || sql->open(sql, DATABASE_PATH) < 0) {
        ALERT("数据库文件丢失!");
        HTML_GOTO("index.html");
        return -1;
    }

    snprintf(sqlstr, sizeof(sqlstr), "select name from user where name=\"%s\";", input);
    if (sql->get_data(sql, sqlstr, sql_username) < 0 || user_right == 0) {
        ALERT("Username Error!");
        HTML_GOTO("index.html");
        return -1;
    }
    return 0;
}

int get_username(char *input, char *errmsg)
{
    ALERT("%s", input);
    return 0;
}

static int passwd_right = 0;
static int sql_passwd(void *arg, int cnt, char **value, char **name)
{
    passwd_right = 1;
    if (!strcmp(value[0], "0")) {
        ALERT("你已被管理员禁止访问，请联系管理员!");
        HTML_GOTO("index.html");
        return -1;
    }

    return 0;
}

int set_password(char *input, char *errmsg)
{
    sqlite_t *sql = sqlite_create();
    char sqlstr[128] = {0};

    if (access(DATABASE_PATH, R_OK) || sql->open(sql, DATABASE_PATH) < 0) {
        ALERT("数据库文件丢失!");
        return -1;
    }

    snprintf(sqlstr, sizeof(sqlstr), "select passwd,permission from user where name=\"%s\" and passwd=\"%s\";", user_name, input);
    if (sql->get_data(sql, sqlstr, sql_passwd) || passwd_right == 0) {
        ALERT("Password Error!");
        HTML_GOTO("index.html");
        return -1;
    }
    return 0;
}

int action_login(char *outbuf, char *errmsg, cgi_func_tab_t *func_tab)
{
     
    return 0;
}

