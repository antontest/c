#ifndef __SQLITE_H__
#define __SQLITE_H__
#include <sqlite3/sqlite3.h>

typedef enum sqlite_state_t sqlite_state_t;
enum sqlite_state_t {
    SQLITE_CLOSED = -1,
    SQLITE_OPENED = 0
};
typedef int (*sql_cb_t) (void *para, int column_cnt, char **column_value, char **column_name);

typedef struct sqlite_t sqlite_t;
struct sqlite_t {
    /**
     * @brief open sqlite3 database
     *
     * @param db   sqlite3 database path
     */
    int (*open) (sqlite_t *this, char *db);

    /**
     * @brief exec sql command
     *
     * @param sql  sqlite sql 
     */
    int (*exec) (sqlite_t *this, char *sql);

    /**
     * @brief gain sql result
     */
    int (*get_data) (sqlite_t *this, char *sql, sql_cb_t callback);
    int (*get_table) (sqlite_t *this, char *sql, int *row, int *col, char ***result);
        
    /**
     * @brief close database
     */
    int (*close) (sqlite_t *this);

    /**
     * @brief destroy instance
     */
    void (*destroy) (sqlite_t *this);

    /**
     * @brief create table
     */
    int (*create_table) (sqlite_t *this, const char *table, const char *column);
};

/**
 * @brief create sqlite_t instance
 */
sqlite_t *sqlite_create();

#endif /* __SQLITE_H__ */
