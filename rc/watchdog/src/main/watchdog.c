/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <proc.h>
#include <utils/utils.h>
#include <utils/enum.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
#define app_name "watchdog"
#define app_file_path "/home/anton/var/app/"
#define app_path "/home/anton/usr/bin/watchdog"
static int app_state()
{
    return app_state_check(app_name, 1, 10, app_file_path app_name, NULL);    
}

static int app_stop()
{
    unlink_state_file_by_name(app_name);
    SYSTEM("killall %s > /dev/null 2>&1", app_name);
    return 0;
}

static int app_start()
{
    if (access(app_path, X_OK)) return -1;
    if (check_app_start_conflict(app_name, app_file_path, app_stop)) return 0;
    if (create_state_file_by_name(app_name) < 0) return -1;
    if (system(app_path " &")) {}

    return 0;
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    /* return value of function main */
    int ret = 0; 

    if (agrc < 2) return 0;
    
    set_app_state_file_path(app_file_path);
    if (!strcmp(agrv[1], "state")) {
        ret = app_state();
        print_app_state(app_name, ret);
    } else if (!strcmp(agrv[1], "start")) {
        ret = app_start();
    } else if (!strcmp(agrv[1], "stop")) {
        ret = app_stop();
    } else if (!strcmp(agrv[1], "restart")) {
        ret = app_stop();
        sleep(1);
        ret = app_start();
    }
    return ret;
}
