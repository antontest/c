#include <sql3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils/utils.h>

typedef struct private_sqlite_t private_sqlite_t;
struct private_sqlite_t {
    /**
     * @brief public interface
     */
    sqlite_t public;

    /**
     * @brief sqlite3 database
     */
    sqlite3 *db;

    /**
     * @brief database state
     */
    sqlite_state_t status;


    /**
     * @brief sqlite3 error message
     */
    char *errmsg;
};
#define sqlite_db  this->db
#define sqlite_err this->errmsg
#define print_err_msg() printf("%s\n", sqlite_err)

METHOD(sqlite_t, open_, int, private_sqlite_t *this, char *db)
{
    int ret = sqlite3_open(db, &sqlite_db);
    if (!ret) this->status = SQLITE_OPENED;
    else if (ret < 0) print_err_msg();
    return ret;
}

METHOD(sqlite_t, exec_, int, private_sqlite_t *this, char *sql)
{
    int ret = 0;
    ret = sqlite3_exec(sqlite_db, sql, NULL, NULL, &sqlite_err);
    if (ret < 0) print_err_msg();
    return ret;
}

/*
static int sql_cb(void *para, int column_cnt, char **column_value, char **column_name)
{
    int i = 0;

    for (i = 0; i < column_cnt; i++) {
        printf("column_name: %s, column_value: %s\n", column_name[i], column_value[i]);
    }
    return 0;
}
*/

METHOD(sqlite_t, get_data_, int, private_sqlite_t *this, char *sql, sql_cb_t callback)
{
    int ret = 0;
    if (!sql) return 0;
    ret = sqlite3_exec(sqlite_db, sql, callback, NULL, &sqlite_err);
    if (ret) print_err_msg();
    return 0;
}

METHOD(sqlite_t, get_table_, int, private_sqlite_t *this, char *sql, int *row, int *col, char **result)
{
    int ret = 0;
    if (!sql) return 0;

    ret = sqlite3_get_table(sqlite_db, sql, &result, row, col, &sqlite_err);
    if (ret) print_err_msg();
    return 0;
}

METHOD(sqlite_t, close_, int, private_sqlite_t *this)
{
    if (sqlite_db) return sqlite3_close(sqlite_db);
    return -1;
}

METHOD(sqlite_t, destroy_, void, private_sqlite_t *this)
{
    if (sqlite_err) sqlite3_free(sqlite_err);
    _close_(this);

    free(this);
}

/**
 * @brief create sqlite_t instance
 */
sqlite_t *sqlite_create()
{
    private_sqlite_t *this;

    INIT(this, 
        .public = {
            .open      = _open_,
            .exec      = _exec_,
            .get_data  = _get_data_,
            .get_table = _get_table_,
            .close     = _close_,
            .destroy   = _destroy_,
        },
        .status = SQLITE_CLOSED,
    );

    return &this->public;
}
