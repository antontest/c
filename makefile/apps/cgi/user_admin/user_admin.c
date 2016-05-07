#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cgi/cgi.h>
#include <sql3/sql3.h>

#define head_div_style "position:absolute;top:%dpx;left:%dpx;width:%dpx;height:%dpx;font-family:\'Arial Negreta\', \'Arial\';font-weight:700;font-style:normal;text-align:center;"
#define head_img_style "position:absolute;top:0px;left:0px;width:%dpx;height:%dpx;"
#define head_text_style "position:absolute;top:7px;left:2px;width:%dpx;word-wrap:break-word;"

#define cell_div_style "position:absolute;top:%dpx;left:%dpx;width:%dpx;height:%dpx;text-align:center;"
#define cell_img_style "position:absolute;top:0px;left:0px;width:%dpx;height:%dpx;"
#define cell_text_style "position:absolute;top:10px;left:2px;width:%dpx;word-wrap:break-word;"

#define table_cell(id, pic_id, content, top, left, width, height, class) \
    do { \
        printf("<div id=\"u%d\" class=\"ax_table_cell\" style=\"" cell_div_style "\">\n", id, top, left, width, height); \
        printf("<img id=\"u%d_img\" class=\"img \" src=\"images/user_admin_page/u%d.png\" style=\"" cell_img_style "\"/>\n", id, pic_id, width, height); \
        printf("<div id=\"u%d\" class=\"text\" style=\"" cell_text_style "\">\n", id + 1, width - 4); \
        printf("<p><span class=\"%s\">%s</span></p>", class, content); \
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

static void user_admin_add_btn(int top)
{
    printf("<div id=\"add_button\" class=\"ax_shape\" title=\"点击新增用户。在上表的用户名（例如super_man）中点击可以修改或删除该用户信息\" style=\"" cell_div_style "\" >\n", top, 626, 80, 30);
    printf("<img id=\"user_admin_add_btn_img\" class=\"img \" src=\"images/user_admin_page/u54.png\" style=\"" cell_img_style"\"/>\n", 80, 30);
    printf("<div id=\"user_admin_add_btn_text\" class=\"text\" style=\"" head_text_style "\">\n", 80 - 4);
    printf("<span>新增</span>\n");
    printf("</div>\n");
    printf("</div>\n");
}
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
    char *class[]  = {"ax_table_name_span", ""};
    char *class_name = NULL;

    col = sizeof(width) / sizeof(width[0]);
    picid = 1;
    for (i = 0; i < col; i++) {
        table_head(i * 2 + 1, i * 2 + picid, colum[i], top, left, width[i], height);
        left += width[i];
    }

    num = cnt - col;
    if (num < 1) {
        goto add_button;
    }

    value  += col;
    top    += height;
    picid   = 21;
    height  = 36;
    for (i = 0; i < num; i++) {
        if (i % col == 0) {
            left = 120;
            class_name = class[0];
            if (i != 0) top += height;
        } else {
            left += width[i % col - 1];
            class_name = class[1];
        }
        table_cell(i * 2 + 1 + 10, (i * 2) % 10 + picid, *value++, top, left, width[i % col], height, class_name);
    }

    num   = i;
    picid = 31;
    left  = 120;
    top   += height;
    for (i = num; i < col + num; i++) {
        if (i % col == 0) {
            class_name = class[0];
        } else {
            class_name = class[1];
        }
        table_cell(i * 2 + 1 + 10, (i * 2) % 10 + picid, *value++, top, left, width[i % col], height, class_name);
        left += width[i % col];
    }

add_button:
    user_admin_add_btn(top + height + 10);
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
    sql3->get_table(sql3, "select user.user_name,role.role_name,user.nick_name,user_status.status_name,user.create_time from user join role,user_status on user.role_id = role.role_id and user.user_status = user_status.status_id;", &row, &col, &value);
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
