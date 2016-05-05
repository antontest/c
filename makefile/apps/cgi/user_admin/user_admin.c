#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cgi/cgi.h>
#include <sql3/sql3.h>

#define DATABASE_PATH  "/home/anton/web/html/db/user.db"
static int user_info(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    sqlite_t *sql3 = NULL;
    
    sql3 = sqlite_create();
    if (!sql3) {
        ALERT("连接数据库失败");
        return -1;
    }

    if (sql3->open(sql3, DATABASE_PATH) < 0) {
        ALERT("连接数据库失败");
        return -1;
    }

    printf("<div id=\"u0\" class=\"ax_table\">\n");
    printf("<table class=\"tbl\" id=\"tabProduct\">\n");
    printf("<tr>\n");
    printf("<th>用户名</th>");
    printf("<th>角色</th>");
    printf("<th>姓名</th>");
    printf("<th>状态</th>");
    printf("<th>创建时间</th>");

    printf("</tr>\n");
    printf("</table>\n");
    printf("</div>");
    sql3->destroy(sql3);
    return 0;
}

int main(int argc, char **argv)
{
    cgi_func_tab_t func[] = {
        {"user_admin_page", KEY_IS_FILE,   NULL,              NULL},
        {"user_info",      KEY_IS_VAR,    (void *)user_info, (void *)user_info},
        {NULL,              KEY_IS_UNKOWN, NULL,              NULL}
    };
    cgi_t *cgi = NULL;

    cgi = cgi_create();
    if (!cgi) {
        return -1;
    }

    CONTENT_TEXT;
    cgi->parse_input(cgi);
    cgi->handle_entry(cgi, func);
    cgi->handle_request(cgi, func);
    cgi->destroy(cgi);

    return 0;
}
