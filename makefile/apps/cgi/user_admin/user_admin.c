#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cgi/cgi.h>
#include <sql3/sql3.h>

//#define cell_div_style(top, left, width, height) "position:absolute;top:" top "px;left:" left "px;width:" width "px;height:" height "font-family:\'Arial Negreta\', \'Arial\';font-weight:700;font-style:normal;text-align:center;"
//#define cell_img_style(width, height) "position:absolute;top:0px;left:0px;width:" width "px;height:" height "px;"
//#define cell_text_style(width) "position:absolute;top:2px;left:7px;width:" width "px;word-wrap:break-word;"

#define head_div_style "position:absolute;top:%dpx;left:%dpx;width:%dpx;height:%dpx;font-family:\'Arial Negreta\', \'Arial\';font-weight:700;font-style:normal;text-align:center;"
#define head_img_style "position:absolute;top:0px;left:0px;width:%dpx;height:%dpx;"
#define head_text_style "position:absolute;top:7px;left:2px;width:%dpx;word-wrap:break-word;"

#define cell_div_style "position:absolute;top:%dpx;left:%dpx;width:%dpx;height:%dpx;text-align:center;"
#define cell_img_style "position:absolute;top:0px;left:0px;width:%dpx;height:%dpx;"
#define cell_text_style "position:absolute;top:10px;left:2px;width:%dpx;word-wrap:break-word;"

#define table_cell(id, pic_id, content, top, left, width, height) \
    do { \
        printf("<div id=\"u%d\" class=\"ax_table_cell\" style=\"" cell_div_style "\">\n", id, top, left, width, height); \
        printf("<img id=\"u%d_img\" class=\"img \" src=\"images/user_admin_page/u%d.png\" style=\"" cell_img_style "\"/>\n", id, pic_id, width, height); \
        printf("<div id=\"u%d\" class=\"text\" style=\"" cell_text_style "\">\n", id + 1, width - 4); \
        printf("<p><span>%s</span></p>", content); \
        printf("</div>\n"); \
        printf("</div>\n"); \
    } while (0)

#define table_head(id, pic_id, content, top, left, width, height) \
    do { \
        printf("<div id=\"u%d\" class=\"ax_table_cell\" style=\"" head_div_style "\">\n", id, top, left, width, height); \
        printf("<img id=\"u%d_img\" class=\"img \" src=\"images/user_admin_page/u%d.png\" style=\"" head_img_style "\"/>\n", id, pic_id, width, height); \
        printf("<div id=\"u%d\" class=\"text\" style=\"" head_text_style "\">\n", id + 1, width - 4); \
        printf("<p><span>%s</span></p>", content); \
        printf("</div>\n"); \
        printf("</div>\n"); \
    } while (0)

static int user_info_cb(int cnt, char **value)
{
    int i          = 0;
    int col        = 0;
    int num        = 0;
    int left       = 120;
    int top        = 100;
    int height     = 30;
    int width[]    = {122, 117, 111, 68, 168};
    int picid      = 1;
    char *colum[]  = {"用户名", "角色", "姓名", "状态", "创建时间"};

    printf("<div id=\"u0\" class=\"ax_table\">\n");

    col = sizeof(width) / sizeof(width[0]);
    value += col;

    picid = 1;
    for (i = 0; i < col; i++) {
        table_head(i * 2 + 1, i * 2 + picid, colum[i], top, left, width[i], height);
        left += width[i];
    }

    num     = cnt - col;
    top    += height;
    picid   = 21;
    height  = 36;
    for (i = 0; i < num; i++) {
        if (i % col == 0) {
            left = 120;
        } else {
            left += width[i % col - 1];
            top += (i / col) * height;
        }
        table_cell(i * 2 + 1 + 10, (i * 2) % 10 + picid, *value++, top, left, width[i % col], height);
    }

    num   = i;
    picid = 31;
    left  = 120;
    top   += height;
    for (i = num; i < col + num; i++) {
        table_cell(i * 2 + 1 + 10, (i * 2) % 10 + picid, *value++, top, left, width[i % col], height);
        left += width[i % col];
    }

    printf("</div>");
    return 0;
}

#define DATABASE_PAtd  "/home/anton/web/html/db/user.db"
static int user_info(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    sqlite_t *sql3 = NULL;
    int row = 0, col = 0;
    char **value = NULL;
    
    sql3 = sqlite_create();
    if (!sql3) {
        ALERT("连接数据库失败");
        return -1;
    }

    if (sql3->open(sql3, DATABASE_PAtd) < 0) {
        ALERT("连接数据库失败");
        return -1;
    }

    //sql3->get_data(sql3, "select user_name,role_id,nick_name,user_status,create_time from user;", user_info_cb);
    sql3->get_table(sql3, "select user.user_name,role.role_name,user.nick_name,user.user_status,user.create_time from user join role on user.role_id = role.role_id;", &row, &col, &value);
    user_info_cb(row * col, value);
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
