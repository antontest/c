/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <utils/utils.h>
#include <data/linked_list.h>
#include <get_args.h>
#include <fileio.h>
#include <signal.h>
#include <proc.h>
#include <utils/log.h>
#include <utils/debug.h>

#define DFT_WDG_CONF_PATH "./wdg_conf.ini"
#define WDG_NAME "watchdog"
#define DFT_APP_TIME_OUT (10)
typedef struct app_pkg_t app_pkg_t;
struct app_pkg_t {
    /**
     * @brief pid of app running
     */
    pid_t pid;

    /**
     * @brief app name
     */
    char *name;

    /**
     * @brief timeout of starting
     */
    int timeout;
};

app_pkg_t *create_app_pkg(const char *name)
{
    app_pkg_t *this;
    
    INIT(this, 
        .name = strdup(name),
    );

    return this;
}

typedef struct wdg_conf_t wdg_conf_t;
struct wdg_conf_t {
    const char *conf_path;
    const char *name;
    unsigned  int  chk_period;
    linked_list_t  *app_list;
    ipc_t *sig_hd;
} wdg_conf = {
    .conf_path = DFT_WDG_CONF_PATH,
    .name      = WDG_NAME,
    .app_list  = NULL,
};

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
/**
 * @brief read config parameters 
 *
 * @return 0, if succ; -1, if failed 
 */
int read_config()
{
    int ret = -1;
    int i = 0;
    char *period = NULL;
    char *app_list = NULL;
    char *app_timeout = NULL;
    char *token, *save, *str;
    app_pkg_t *app = NULL;
    ini_t *cfg = NULL;

    cfg = create_ini(wdg_conf.conf_path);
    if (!cfg) return -1;

    period = cfg->get_value(cfg, "period", "period");
    if (!period) {
        dbg("read config period error");
        goto over;
    }
    wdg_conf.chk_period = atoi(period);
    if (wdg_conf.chk_period < 1) goto over;

    app_list = cfg->get_value(cfg, "app", "list");
    if (!app_list) {
        dbg("read config app list error");
        goto over;
    }
    wdg_conf.app_list = linked_list_create();
    if (!wdg_conf.app_list) goto over;

    for (str = app_list; ; str = NULL) {
        token = strtok_r(str, ";,", &save);
        if (token == NULL) break;

        app = create_app_pkg(token);
        wdg_conf.app_list->insert_last(wdg_conf.app_list, app);
    }

    app_timeout = cfg->get_value(cfg, "app", "timeout");
    wdg_conf.app_list->reset_current(wdg_conf.app_list);
    for (str = app_timeout, i = 0; i < wdg_conf.app_list->get_count(wdg_conf.app_list); str = NULL, i++) {
        token = strtok_r(str, ";,", &save);
        if (!token) break;

        wdg_conf.app_list->get_next(wdg_conf.app_list, (void **)&app);
        app->timeout = atoi(token);
        if (app->timeout < 1) app->timeout = DFT_APP_TIME_OUT;
    }
    for (; i < wdg_conf.app_list->get_count(wdg_conf.app_list); i++) {
        app->timeout = DFT_APP_TIME_OUT;
    }

    ret = 0;
over:
    if (cfg != NULL) cfg->destroy(cfg);

    return ret;
}

/**
 * @brief watchdog deinit and free memory 
 */
void wdg_deinit()
{
    app_pkg_t *app = NULL;

    if (wdg_conf.sig_hd != NULL) wdg_conf.sig_hd->destroy(wdg_conf.sig_hd);
    if (!wdg_conf.app_list) return;
    wdg_conf.app_list->reset_current(wdg_conf.app_list);
    while (wdg_conf.app_list->remove_first(wdg_conf.app_list, (void **)&app) != NOT_FOUND) {
        if (app->name != NULL) free(app->name);
        free(app);
    }
    free(wdg_conf.app_list);
}

/**
 * @brief error handler 
 */
void error_handler(int sig, siginfo_t *info, void *text)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
        case SIGKILL:
        case SIGSTOP:
            wdg_deinit();
            exit(1);
            break;
    }
}

static int app_state(const char *name)
{
    char cmd[64] = {0};

    snprintf(cmd, sizeof(cmd), "/home/anton/usr/bin/rc_%s", name);
    if (access(cmd, X_OK)) return APP_NOAPP;
    snprintf(cmd, sizeof(cmd), "/home/anton/usr/bin/rc_%s state", name);
    return WEXITSTATUS(system(cmd));
}

#define APP_PID_FILE_PATH "/home/anton/var/app/"
void app_state_chk()
{
    int app_cnt = 0;
    int status = 0;
    app_pkg_t *app = NULL;

    app_cnt = wdg_conf.app_list->get_count(wdg_conf.app_list);
    wdg_conf.app_list->reset_current(wdg_conf.app_list);
    while (app_cnt-- > 0) {
        wdg_conf.app_list->get_next(wdg_conf.app_list, (void **)&app);
        status = app_state(app->name);
        dbg("[%s] %s", app->name, app_state_string(status));
    }
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    /* return value of function main */
    int ret = -1; 
    struct timeval tv = {0};

    if (!check_proc_unique(WDG_NAME)) goto over;
    if (read_config() < 0) goto over;
    dbg("read config succ");

    wdg_conf.sig_hd = create_ipc();
    if (!wdg_conf.sig_hd) goto over;
    wdg_conf.sig_hd->mksig(wdg_conf.sig_hd, error_handler);
    wdg_conf.sig_hd->sigact(wdg_conf.sig_hd, SIGINT);
    wdg_conf.sig_hd->sigact(wdg_conf.sig_hd, SIGTERM);
    wdg_conf.sig_hd->sigact(wdg_conf.sig_hd, SIGKILL);
    wdg_conf.sig_hd->sigact(wdg_conf.sig_hd, SIGSTOP);

    while (1) {
        tv.tv_sec = wdg_conf.chk_period / 1000;
        tv.tv_usec = wdg_conf.chk_period % 1000 * 1000;

        select(0, NULL, NULL, NULL, &tv);
        app_state_chk();   
    }

over:
    wdg_deinit();
    return ret;
}
