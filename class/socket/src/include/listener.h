#ifndef __LISTENER_H__
#define __LISTENER_H__
#include "socket_common.h"

typedef void (*listener_handler) (int fd, void *arg);
typedef struct listener_cb_t listener_cb_t;
struct listener_cb_t {
    listener_handler handler;
    void *arg;
};

typedef struct listener_t listener_t;
struct listener_t {
    /**
     * @brief create tcp or udp server
     * @return socket fd
     */
    int (*listen) (listener_t *this, int family, int type, char *ip, int port);

    /**
     * @brief create tcp or udp client
     * @return socket fd
     */
    int (*connect) (listener_t *this, int family, int type, char *ip, int port);

    /**
     * @brief close socket
     */
    int (*close) (listener_t *this);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (listener_t *this);

    /**
     * @brief set socket event callback
     */
    void (*set_cb) (listener_t *this, on_type_t type, listener_handler handler, void *arg);

    /**
     * @brief deal with socket when accepting
     */
    listener_cb_t *on_accept;

    /**
     * @brief deal with socket when connected succ
     */
    listener_cb_t *on_connect;

    /**
     * @brief deal with socket when recved message
     */
    listener_cb_t *on_recv;

    /**
     * @brief deal with socket when closed
     */
    listener_cb_t *on_close;
};

/**
 * @brief create_tcp_listener 
 */
listener_t *create_listener();

/**
 * @brief create_listener_cb 
 */
listener_cb_t *create_listener_cb(void (*handler) (int , void *), void *arg);

#endif /* __LISTENER_H__ */
