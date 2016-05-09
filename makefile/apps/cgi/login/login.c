#include <cgi/cgi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sql3/sql3.h>
#include <../cgi.h>

#define USER_DATA_DB_PATH "/home/anton/web/html/db/user.db"
static int role_id     = -1;
static int user_status = -1;
static int user_login(void *arg, int cnt, char **value, char **name)
{
    role_id = atoi(value[0]);
    user_status = atoi(value[1]);
    if (!user_status) {
        ALERT("你已被管理员禁止访问，请联系管理员!");
        HTML_GOTO("index.html");
        return -1;
    } else if (user_status == 1) {
        ALERT("账号异常，请联系管理员!");
        HTML_GOTO("index.html");
        return -1;
    }

    return 0;
}

void handle_login()
{
    sqlite_t *sql    = NULL;
    char sqlstr[128] = {0};
    char *uid        = find_value("uid");
    char *pwd        = find_value("pwd");

    if (!uid || !pwd) {
        ALERT("用户名或者密码错误!");
        return;
    }

    sql = sqlite_create();
    if (access(USER_DATA_DB_PATH, R_OK) || sql->open(sql, USER_DATA_DB_PATH) < 0) {
        ALERT("数据库文件丢失!");
        HTML_GOTO("index.htm");
        return;
    }

    snprintf(sqlstr, sizeof(sqlstr), "select role_id,user_status from user where user_name=\"%s\" and user_pwd=\"%s\";", uid, pwd);
    if (sql->get_data(sql, sqlstr, user_login) < 0 || role_id < 0) {
        ALERT("用户名或者密码错误!");
        HTML_GOTO("index.htm");
        return;
    }
    
    if (role_id == 0) {
        HTML_GOTO("admin_page.html");
    } else {
        HTML_GOTO("ftp.html");
    }

    return ;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    cgi_t *cgi = NULL;

    cgi_header_content_type("text/html");
    HTML_UTF8();
    //printf("<META  http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
    cgi = cgi_create();
    cgi->parse_input(cgi);
    handle_login();
    cgi->destroy(cgi);

    return rt;
}
