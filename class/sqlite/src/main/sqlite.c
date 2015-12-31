/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
static int cb(void *arg, int cnt, char **value, char **name)
{
    int i = 0;

    for (i = 0; i < cnt; i++) {
        printf("name: %s, value: %s\n", name[i], value[i]);
    }
    return 0;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    sqlite_t *sql = sqlite_create();

    sql->open(sql, "test.db");
    //sql->exec(sql, "create table tb(id INTEGER PRIMARY KEY, data TEXT)");
    //sql->exec(sql, "insert into tb values (1, \"init sqlite3\")");
    sql->get_data(sql, "select * from tb;", cb);
    sql->destroy(sql);

    return rt;
}
