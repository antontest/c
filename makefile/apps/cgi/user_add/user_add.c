#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sql3/sql3.h>
#include <cgi/cgi.h>
#include <time.h>

#define DATABASE_PATH "/home/anton/web/html/db/user.db"
static int role_name_opt(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    int row        = 0;
    int col        = 0;
    int num        = 0;
    char **value   = NULL;
    sqlite_t *sql3 = NULL;

    sql3 = sqlite_create();
    if (!sql3 || sql3->open(sql3, DATABASE_PATH) < 0) {
        return -1;
    }

    sql3->get_table(sql3, "select role_name from role order by role_id desc;", &row, &col, &value);
    if (row > 1 && col > 0 && value) {
        value += col;
        num = row * col;
        while (num-- > 0) {
            /*
            if (!strcmp(*value, "访客")) {
                printf("<option value=\"%s\" selected=\"selected\">%s</option>\n", *value, *value);
            } else {
                printf("<option value=\"%s\">%s</option>\n", *value, *value);
            }
            */
            printf("<option value=\"%s\">%s</option>\n", *value, *value);
            value++;
        }
        
    }

    sql3->destroy(sql3);
    return 0;
}

static int user_status_opt(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    int row        = 0;
    int col        = 0;
    int num        = 0;
    char **value   = NULL;
    sqlite_t *sql3 = NULL;

    sql3 = sqlite_create();
    if (!sql3 || sql3->open(sql3, DATABASE_PATH) < 0) {
        return -1;
    }

    sql3->get_table(sql3, "select status_name from user_status;", &row, &col, &value);
    if (row > 1 && col > 0 && value) {
        value += col;
        num = row * col;
        while (num-- > 0) {
            printf("<option value=\"%s\">%s</option>\n", *value, *value);
            value++;
        }
    }

    sql3->destroy(sql3);
    return 0;
}

static int add_user_to_db(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    enum data_info_t {
        USER_NAME = 0,
        USER_PWD,
        NICK_NAME,
        ROLE_NAME,
        USER_STATUS,
        EMAIL,
    };
    struct user_info {
        char *name;
        char *value;
    } data[] = {
        {"user_name"},
        {"user_pwd"},
        {"nick_name"},
        {"role_name"},
        {"user_status"},
        {"email"},
        {NULL}
    };
    struct user_info *p = NULL;
    char str[1024] = {0};
    char stime[56] = {0};
    char **value   = NULL;
    int row = 0;
    int col = 0;
    time_t t;
    struct tm *tmp;
    sqlite_t *sql3 = NULL;

    for (p = data; p && p->name; p++) {
        p->value = find_value(p->name);
        if (!p->value) {
            ALERT("用户信息输入有误!");
            return -1;
        }
        //ALERT("%s", p->value);
    }

    sql3 = sqlite_create();
    if (!sql3 || sql3->open(sql3, DATABASE_PATH) < 0) {
        return -1;
    }
    
    snprintf(str, sizeof(str), "select 1 from user where user_name=\"%s\";", data[USER_NAME].value);
    sql3->get_table(sql3, str, &row, &col, &value);
    if (row > 0 && col > 0 && value) {
        ALERT("用户名已经存在!");
        HTML_GOTO("user_add.cgi?next_file=user_add.html");
        return -1;
    }

    t = time(NULL);
    tmp = localtime(&t);
    if (tmp) {
        strftime(stime, sizeof(stime), "20%y-%m-%d %H:%M:%S", tmp);
    }
    snprintf(str, sizeof(str), "insert into user (user_id, user_name, user_pwd, nick_name, role_id, user_status, email, create_time) values ((select max(user_id)+1 from user), \"%s\", \"%s\", \"%s\", (select role_id from role where role.role_name=\"%s\"), (select status_id from user_status where user_status.status_name=\"%s\"), \"%s\", \"%s\");", 
            data[USER_NAME].value, 
            data[USER_PWD].value, 
            data[NICK_NAME].value, 
            data[ROLE_NAME].value,
            data[USER_STATUS].value,
            data[EMAIL].value,
            stime);
    //ALERT("%s", str);
    sql3->exec(sql3, str);
    sql3->destroy(sql3);

    HTML_GOTO("user_admin.cgi?next_file=user_admin_page.html");

    return 0;
}

int main(int argc, char **argv)
{
    cgi_func_tab_t func[] = {
        {"user_add", KEY_IS_FILE, NULL, NULL},
        {"role_name_opt", KEY_IS_VAR, (void *)role_name_opt, NULL},
        {"user_status_opt", KEY_IS_VAR, (void *)user_status_opt, NULL},
        {"user_name", KEY_IS_VAR, NULL, (void *)add_user_to_db},
        {NULL}
    };
    cgi_t *cgi = NULL;

    cgi = cgi_create();
    if (!cgi) {
        return -1;
    }

    CONTENT_TEXT;
    HTML_UTF8();
    cgi->parse_input(cgi);
    cgi->handle_entry(cgi, func);
    cgi->handle_request(cgi, func);
    cgi->destroy(cgi);
    return 0;
}
