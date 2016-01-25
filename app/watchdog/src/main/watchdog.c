/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <utils/linked_list.h>
#include <utils/get_args.h>
#include <fileio.h>
#include <signal.h>
#include <proc.h>
#include <utils/log.h>
#include <utils/debug.h>
#include <socket/socket.h>
#include <socket/message.h>
#include <thread/thread.h>
#include <thread/mutex.h>

#define APP_PID_FILE_PATH "/home/anton/var/app/"
#define DFT_WDG_CONF_PATH "/home/anton/usr/bin/wdg_conf.ini"
#define RC_BIN_PATH "/home/anton/usr/bin/"
#define DFT_WDG_LOG_FILE  NULL
#define WDG_NAME "watchdog"
#define DFT_APP_TIME_OUT (10)

static char *default_app_pid_file_path = APP_PID_FILE_PATH;
static char *default_wdg_conf_path = DFT_WDG_CONF_PATH;
static char *default_wdg_log_file = DFT_WDG_LOG_FILE;
static char *default_rc_bin_path  = RC_BIN_PATH;

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

    /**
     * restart flag
     */
    int is_restart;
};

app_pkg_t *create_app_pkg(const char *name, int timeout, int is_restart)
{
    app_pkg_t *this;
    
    INIT(this, 
        .name = strdup(name),
        .timeout = timeout,
        .is_restart = is_restart,
    );

    return this;
}

typedef struct wdg_conf_t wdg_conf_t;
struct wdg_conf_t {
    const char *name;
    unsigned  int  chk_period;
    linked_list_t  *app_list;
    ipc_t *sig_hd;
    msg_mod_t *wdg_mod;
    mutex_t *app_lock;
} wdg_conf = {
    .name      = WDG_NAME,
    .app_list  = NULL,
    .wdg_mod   = NULL,
    .sig_hd    = NULL,
    .app_lock  = NULL
};

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
/**
 * @brief handle message
 */
static void msg_handler(struct msg_t *msg)
{
    wdg_app_info_t *app_info = NULL;
    app_pkg_t *app_pkg = NULL; 

    printf("wdg msg comming\n");
    switch (msg->msg_id) {
        case MSG_ID_APP_CHECK_ADD:
            printf("add app check\n");
            app_info = (wdg_app_info_t *)msg->data;
            app_pkg = create_app_pkg(app_info->name, app_info->timeout, app_info->is_restart);
            if (!app_pkg) return;
            printf("add app check\n");
            wdg_conf.app_lock->lock(wdg_conf.app_lock);
            wdg_conf.app_list->insert_last(wdg_conf.app_list, app_pkg);
            wdg_conf.app_lock->unlock(wdg_conf.app_lock);
            break;
        case MSG_ID_APP_CHECK_DEL:
            break;
        default:
            break;
    }
}

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
    char *app_restart = NULL;
    char *token, *save, *str;
    app_pkg_t *app = NULL;
    ini_t *cfg = NULL;

    cfg = create_ini(default_wdg_conf_path);
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

        app = create_app_pkg(token, 0, 0);
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

    app_restart = cfg->get_value(cfg, "app", "restart");
    wdg_conf.app_list->reset_current(wdg_conf.app_list);
    for (str = app_restart, i = 0; i < wdg_conf.app_list->get_count(wdg_conf.app_list); str = NULL, i++) {
        token = strtok_r(str, ";,", &save);
        if (!token) break;

        wdg_conf.app_list->get_next(wdg_conf.app_list, (void **)&app);
        app->is_restart = atoi(token);
        if (app->is_restart < 0) app->timeout = 0;
    }
    for (; i < wdg_conf.app_list->get_count(wdg_conf.app_list); i++) {
        app->is_restart = 0;
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

    if (wdg_conf.app_lock) {
        wdg_conf.app_lock->unlock(wdg_conf.app_lock);
        usleep(100);
        wdg_conf.app_lock->destroy(wdg_conf.app_lock);
    }
    if (wdg_conf.wdg_mod) wdg_conf.wdg_mod->destroy(wdg_conf.wdg_mod);
    if (wdg_conf.sig_hd != NULL) wdg_conf.sig_hd->destroy(wdg_conf.sig_hd);
    if (!wdg_conf.app_list) return;
    wdg_conf.app_list->reset_current(wdg_conf.app_list);
    while (wdg_conf.app_list->remove_first(wdg_conf.app_list, (void **)&app) != NOT_FOUND) {
        if (app->name != NULL) free(app->name);
        free(app);
    }
    log_deinit();
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

/**
 * @brief watchdog init 
 */
static int wdg_init()
{
    mod_cfg_t cfg = {0};

    /**
     * init lock
     */
    wdg_conf.app_lock = mutex_create();
    if (!wdg_conf.app_lock) {
        log_notice0(DBG_WDG, "create mutex failed");
        return -1;
    }

    /**
     * register watchdog message recving module
     */
    wdg_conf.wdg_mod = create_msg_mod();
    if (!wdg_conf.wdg_mod) {
        log_notice0(DBG_WDG, "create watchdog message module failed");
        return -1;
    }
    cfg.name = "watchdog";
    cfg.handler = (void *)msg_handler;
    if (wdg_conf.wdg_mod->act(wdg_conf.wdg_mod, &cfg) < 0) {
        log_notice0(DBG_WDG, "register watchdog module failed");
        return -1;
    }

    /**
     * create ipc
     */
    wdg_conf.sig_hd = create_ipc();
    if (!wdg_conf.sig_hd) {
        log_notice0(DBG_WDG, "create ipc failed");
        return -1;
    }
    wdg_conf.sig_hd->mksig(wdg_conf.sig_hd, error_handler);
    wdg_conf.sig_hd->sigact(wdg_conf.sig_hd, SIGINT);
    wdg_conf.sig_hd->sigact(wdg_conf.sig_hd, SIGTERM);
    wdg_conf.sig_hd->sigact(wdg_conf.sig_hd, SIGKILL);
    wdg_conf.sig_hd->sigact(wdg_conf.sig_hd, SIGSTOP);

    return 0;
}

/**
 * @brief check apps state
 */
static int app_state(const char *name)
{
    char cmd[64] = {0};

    snprintf(cmd, sizeof(cmd), "%src_%s", default_rc_bin_path, name);
    if (access(cmd, X_OK)) return APP_NOAPP;
    snprintf(cmd, sizeof(cmd), "%src_%s state > /dev/null", default_rc_bin_path, name);
    return WEXITSTATUS(system(cmd));
}

/**
 * @brief app_state_chk 
 */
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
        log_notice0(DBG_WDG, "[%s] %s", app->name, app_state_string(status));

        switch (status) {
            case APP_CRASHED:
                if (app->is_restart) SYSTEM("%s restart", app->name);
                break;
            default:
                break;
        }
    }
}

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    /* return value of function main */
    int ret = -1; 
    int help_flag = 0;
    struct timeval tv = {0};
    char *wdg_conf_path = NULL;
    char *wdg_log_file  = NULL;
    char *app_pid_file_path = NULL;
    char *rc_bin_path = NULL;
    struct options opts[] = {
        {"-h", "--help",      0, RET_INT, ADDR_ADDR(help_flag)},
        {"-c", "--conf_path", 1, RET_STR, ADDR_ADDR(wdg_conf_path)},
        {"-l", "--log_path",  1, RET_STR, ADDR_ADDR(wdg_log_file)},
        {"-p", "--pid_path",  1, RET_STR, ADDR_ADDR(app_pid_file_path)},
        {"-r", "--rc_path",   1, RET_STR, ADDR_ADDR(rc_bin_path)},
        {NULL, NULL}
    };
    struct usage usg[] = {
        {"-h, --help",      "Show usage"},
        {"-c, --conf_path", "path of config file"},
        {"-l, --log_path",  "path of watchdog log file saving"},
        {"-p, --pid_path",  "path of apps pid file saving"},
        {"-r, --rc_path",   "path of apps rc bin"},
        {NULL, NULL}
    };

    /**
     * parser parameters
     */
    get_args(agrc, agrv, opts);
    if (help_flag) {
        print_usage(usg);
        goto over;
    }
    if (wdg_conf_path) default_wdg_conf_path = wdg_conf_path;
    if (wdg_log_file) default_wdg_log_file = wdg_log_file;
    if (app_pid_file_path) default_app_pid_file_path = app_pid_file_path;
    if (rc_bin_path) default_rc_bin_path = rc_bin_path;

    /**
     * 1. check_proc_unique 
     * 2. read config 
     * 3. create log instance
     */
    if (log_init(default_wdg_log_file) < 0) goto over;
    if (!check_proc_unique(WDG_NAME)) goto over;
    if (read_config() < 0) {
        log_notice0(DBG_WDG, "read config failed");
        goto over;
    }
    log_notice0(DBG_WDG, "read config successfully");

    /**
     * watchdog init
     */
    wdg_init();

    /**
     * check apps state cyclely
     */
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
