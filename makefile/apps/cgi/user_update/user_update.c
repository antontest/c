#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sql3/sql3.h>
#include <cgi/cgi.h>
#include <time.h>

#define DATABASE_PATH "/home/anton/web/html/db/user.db"
static int user_update()
{
    enum {
        USER_PWD = 0,
        NICK_NAME,
        ROLE_NAME,
        USER_STATUS,
        EMAIL
    };
    struct user_data {
        char *name;
        char *value;
    } user[] = {
        {"user_pwd"},
        {"nick_name"},
        {"role_name"},
        {"user_status"},
        {"email"},
        {NULL}
    };
    struct user_data *p = NULL;
    int len           = 0;
    char *user_name   = NULL;
    char strsql[512]  = {0};
    char update[256]  = {0};
    sqlite_t *sql3    = NULL;

    /**
     * gain user name
     */
    user_name = find_value("user_name");
    if (!user_name) {
        return -1;
    }

    p = user;
    while (p && p->name) {
        p->value = find_value(p->name);
        p++;
    }

    if (user[USER_PWD].value) {
        len += snprintf(update + len, sizeof(update) - len - 1, "%s=\"%s\",", user[USER_PWD].name, user[USER_PWD].value);
    }
    if (user[NICK_NAME].value) {
        len += snprintf(update + len, sizeof(update) - len - 1, "%s=\"%s\",", user[NICK_NAME].name, user[NICK_NAME].value);
    }
    if (user[EMAIL].value) {
        len += snprintf(update + len, sizeof(update) - len - 1, "%s=\"%s\",", user[EMAIL].name, user[EMAIL].value);
    }
    if (user[USER_STATUS].value) {
        len += snprintf(update + len, sizeof(update) - len - 1, "%s=(select status_id from user_status where status_name=\"%s\"),", user[USER_STATUS].name, user[USER_STATUS].value);
    }
    if (user[ROLE_NAME].value) {
        len += snprintf(update + len, sizeof(update) - len - 1, "%s=(select role_id from role where role_name=\"%s\"),", "role_id", user[ROLE_NAME].value);
    }
    update[len - 1] = '\0';

    sql3 = sqlite_create();
    if (!sql3 || sql3->open(sql3, DATABASE_PATH) < 0) {
        return -1;
    }

    snprintf(strsql, sizeof(strsql), "update user set %s where user.user_name=\"%s\";", update, user_name);
    //ALERT("%s", strsql);
    sql3->exec(sql3, strsql);
    sql3->destroy(sql3);
    HTML_GOTO("user_admin.cgi?next_file=user_admin_page.html");
    return 0;
}

int main(int argc, char **argv)
{
    cgi_func_tab_t func[] = {
        {"user_detail", KEY_IS_FILE, NULL, NULL},
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
    user_update();
    cgi->handle_entry(cgi, func);
    cgi->destroy(cgi);

    return 0;
}
