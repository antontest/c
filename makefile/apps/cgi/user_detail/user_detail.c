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
    int row            = 0;
    int col            = 0;
    int num            = 0;
    char **value       = NULL;
    char *user_name    = NULL;
    char strsql[256]   = {0};
    char role_name[56] = {0};
    sqlite_t *sql3     = NULL;

    user_name = find_value("user_name");
    if (!user_name) {
        return -1;
    }

    sql3 = sqlite_create();
    if (!sql3 || sql3->open(sql3, DATABASE_PATH) < 0) {
        return -1;
    }

    snprintf(strsql, sizeof(strsql), "select role_name from role where role_id=(select role_id from user where user_name=\"%s\");", user_name);
    sql3->get_table(sql3, strsql, &row, &col, &value);
    if (row > 0 && col > 0 && value) {
        value += col;
        strncpy(role_name, *value, sizeof(role_name));
        printf("<option value=\"%s\">%s</option>\n", *value, *value);
    }

    snprintf(strsql, sizeof(strsql), "select role_name from role  where role_name != \"%s\" order by role_name desc;", role_name);
    sql3->get_table(sql3, strsql, &row, &col, &value);
    if (row > 0 && col > 0 && value) {
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

static int user_status_opt(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    int row              = 0;
    int col              = 0;
    int num              = 0;
    char *user_name      = NULL;
    char **value         = NULL;
    char strsql[256]     = {0};
    char user_status[56] = {0};
    sqlite_t *sql3       = NULL;

    user_name = find_value("user_name");
    if (!user_name) {
        return -1;
    }

    sql3 = sqlite_create();
    if (!sql3 || sql3->open(sql3, DATABASE_PATH) < 0) {
        return -1;
    }

    snprintf(strsql, sizeof(strsql), "select status_name from user_status where status_id=(select user_status from user where user_name=\"%s\");", user_name);
    sql3->get_table(sql3, strsql, &row, &col, &value);
    if (row > 0 && col > 0 && value) {
        value += col;
        strncpy(user_status, *value, sizeof(user_status) - 1);
        printf("<option value=\"%s\">%s</option>\n", *value, *value);
    }

    snprintf(strsql, sizeof(strsql), "select status_name from user_status where status_name!=\"%s\";", user_status);
    sql3->get_table(sql3, strsql, &row, &col, &value);
    if (row > 0 && col > 0 && value) {
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

/*
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
    sqlite_t *sql3 = NULL;
    char str[1024] = {0};
    char stime[56] = {0};
    time_t t;
    struct tm *tmp;

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
    ALERT("%s", str);
    sql3->exec(sql3, str);
    sql3->destroy(sql3);

    HTML_GOTO("user_admin.cgi?next_file=user_admin_page.html");

    return 0;
}
*/

static int user_name(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    char *user_name = NULL;

    user_name = find_value("user_name");
    if (user_name) {
        printf("%s", user_name);
    }

    return 0;
}

static int user_pwd(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    sqlite_t *sql3 = NULL;
    char **value;
    char str[128] = {0};
    int row = 0;
    int col = 0;

    sql3 = sqlite_create();
    if (!sql3 || sql3->open(sql3, DATABASE_PATH) < 0) {
        return -1;
    }

    snprintf(str, sizeof(str), "select user_pwd from user where user.user_name=\"%s\";", find_value("user_name"));
    sql3->get_table(sql3, str, &row, &col, &value);
    if (row > 0 && col > 0 && value) {
        value++;
        printf("%s", *value);
    }

    sql3->destroy(sql3);
    return 0;
}

static int nick_name(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    sqlite_t *sql3 = NULL;
    char **value;
    char str[128] = {0};
    int row = 0;
    int col = 0;

    sql3 = sqlite_create();
    if (!sql3 || sql3->open(sql3, DATABASE_PATH) < 0) {
        return -1;
    }

    snprintf(str, sizeof(str), "select nick_name from user where user.user_name=\"%s\";", find_value("user_name"));
    sql3->get_table(sql3, str, &row, &col, &value);
    if (row > 0 && col > 0 && value) {
        value++;
        printf("%s", *value);
    }

    sql3->destroy(sql3);
    return 0;
}

static int email(char *outbuf, char *errbuf, cgi_form_entry_t *entry)
{
    sqlite_t *sql3 = NULL;
    char **value;
    char str[128] = {0};
    int row = 0;
    int col = 0;

    sql3 = sqlite_create();
    if (!sql3 || sql3->open(sql3, DATABASE_PATH) < 0) {
        return -1;
    }

    snprintf(str, sizeof(str), "select email from user where user.user_name=\"%s\";", find_value("user_name"));
    sql3->get_table(sql3, str, &row, &col, &value);
    if (row > 0 && col > 0 && value) {
        value++;
        printf("%s", *value);
    }

    sql3->destroy(sql3);
    return 0;
}


int main(int argc, char **argv)
{
    cgi_func_tab_t func[] = {
        {"user_detail", KEY_IS_FILE, NULL, NULL},
        {"role_name_opt", KEY_IS_VAR, (void *)role_name_opt, NULL},
        {"user_status_opt", KEY_IS_VAR, (void *)user_status_opt, NULL},
        {"user_name", KEY_IS_VAR, (void *)user_name, NULL},
        {"user_pwd", KEY_IS_VAR, (void *)user_pwd, NULL},
        {"nick_name", KEY_IS_VAR, (void *)nick_name, NULL},
        {"email", KEY_IS_VAR, (void *)email, NULL},
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
