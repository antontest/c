#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include "socket_common.h"

typedef struct msg_t msg_t;
struct msg_t {
    /**
     * @brief this module belong to
     */
    mod_id_t src_mod;

    /**
     * @brief destine module belong to
     */
    mod_id_t dst_mod;

    /**
     * @brief message id
     */
    msg_id_t msg_id;

    /**
     * @brief length of message
     */
    unsigned int msg_len;

    /**
     * @brief content of message
     */
    char data[0];
};

typedef struct mod_cfg_t mod_cfg_t;
struct mod_cfg_t {
    /**
     * @brief module id
     */
    mod_id_t id;

    /**
     * @brief module name
     */
    const char *name;

    /**
     * @brief message callback function
     */
    void (*handler) (msg_t *msg);
};

typedef struct msg_mod_t msg_mod_t;
struct msg_mod_t {
    /**
     * @brief register module
     */
    int (*act) (msg_mod_t *this, mod_cfg_t *cfg);
    
    /**
     * @brief destroy instance and free memory 
     */
    void (*destroy) (msg_mod_t *this);
};

/**
 * @brief create msg_mod instance 
 */
msg_mod_t *create_msg_mod();

typedef struct msg_handler_t msg_handler_t;
struct msg_handler_t {
    /**
     * @brief send message
     */
    int (*send) (msg_handler_t *this, msg_t *msg);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (msg_handler_t *this);
};

/**
 * @brief create msg_handler instance 
 */
msg_handler_t *create_msg_handler();

/**
 * @brief create new_msg 
 *
 * @param src_mod source module id
 * @param dst_mod destine module id 
 * @param msg_id  message id 
 * @param data    message body data 
 * @param size    size of message body
 */
msg_t *create_new_msg(mod_id_t src_mod, mod_id_t dst_mod, msg_id_t msg_id, void *data, int size);

/**
 * @brief create_new_wdg_msg 
 *
 * @param src_mod     source module id
 * @param dst_mod     destine module id 
 * @param msg_id      message id 
 * @param name        app name 
 * @param timeout     app starting timeout 
 * @param is_restart  is restart when app crashed
 */
msg_t *create_new_wdg_msg(mod_id_t src_mod, mod_id_t dst_mod, msg_id_t msg_id, const char *name, int timeout, int is_restart);

#endif /* __MESSAGE_H__ */
