#ifndef __SOCKET_COMMON_H__
#define __SOCKET_COMMON_H__

typedef enum server_type_t server_type_t;
enum server_type_t {
    TYPE_TCP_SERVER = 1 << 1,
    TYPE_TCP_CLIENT = 3 << 1,
    TYPE_UDP_SERVER = 1 << 3,
    TYPE_UDP_CLIENT = 3 << 3
};

typedef enum on_type_t on_type_t;
enum on_type_t {
    ON_ACCEPT = 1,
    ON_CONNECT,
    ON_RECV,
    ON_CLOSE
};

typedef struct wdg_app_info_t wdg_app_info_t;
struct wdg_app_info_t {
    /**
     * app name
     */
    const char *name;
    
    /**
     * @brief timeout of app starting
     */
    int timeout;

    /**
     * @brief flag of whether restart if app crashed
     */
    int is_restart;
} *app_info;

typedef enum task_type_t task_type_t;
enum task_type_t {
    TASK_TYPE_COMMAND = 0,
    TASK_TYPE_SYSN_TIME
};

typedef struct task_msg_t task_msg_t;
struct task_msg_t {
    /**
     * @brief task type
     */
    task_type_t type;

    /**
     * @brief message length
     */
    int len;

    /**
     * @brief task message data 
     */
    char data[0];
};

typedef enum mod_id_t mod_id_t;
enum mod_id_t {
    MOD_ID_UNKNOW = -1,
    MOD_ID_CTL,
    MOD_ID_WDG
};

typedef enum msg_id_t msg_id_t;
enum msg_id_t {
    MSG_ID_UNKNOW = -1,

    /**
     * watchdog message ID
     */
    MSG_ID_APP_CHECK_ADD,
    MSG_ID_APP_CHECK_DEL,
    MSG_ID_TEST
};


#endif /* __SOCKET_COMMON_H__ */
