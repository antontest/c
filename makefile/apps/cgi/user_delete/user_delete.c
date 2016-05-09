#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sql3/sql3.h>
#include <cgi/cgi.h>
#include <time.h>
#include "../cgi_common.h"

static int user_delete()
{
    char *user_name   = NULL;
    char strsql[512]  = {0};
    sqlite_t *sql3    = NULL;

    /**
     * gain user name
     */
    user_name = find_value("user_name");
    if (!user_name) {
        return -1;
    }

    sql3 = sqlite_create();
    if (!sql3 || sql3->open(sql3, USER_DATA_DB_PATH) < 0) {
        return -1;
    }

    snprintf(strsql, sizeof(strsql), "delete from user where user.user_name=\"%s\";", user_name);
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
    user_delete();
    cgi->handle_entry(cgi, func);
    cgi->destroy(cgi);

    return 0;
}
