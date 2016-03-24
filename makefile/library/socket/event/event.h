#ifndef __SOCKET_EVENT__
#define __SOCKET_EVENT__

typedef enum event_type_t event_type_t;
enum event_type_t
{
    EVENT_ON_ACCEPT  = 1     << 1,
    EVENT_ON_CONNECT = 1     << 2,
    EVENT_ON_RECV    = 1     << 3,
    EVENT_ON_CLOSE   = 1     << 4,
    EVENT_ON_ALL     = 0x111 << 1
};

typedef enum event_mode_t event_mode_t;
enum event_mode_t {
    EVENT_MODE_NULL   = -1,
    EVENT_MODE_SELECT = 1,
    EVENT_MODE_EPOLL
};

typedef enum exception_type_t exception_type_t;
enum exception_type_t {
    EXCEPTION_TIMEOUT = 1,
    EXCEPTION_ERROR
};

typedef struct event_t event_t;
struct event_t {
    /**
     * @brief add socket event
     *
     * @param fd        fd listening on
     * @param type      type of listening
     * @param handler   event handler callback
     * @param arg       parameter of callback
     */
    int (*add) (event_t *this, int fd, event_type_t type, void (*handler) (int fd, void *arg), void *arg);

    /**
     * @brief delete socket event
     *
     * @param fd        fd listening on
     * @param type      type of listening
     */
    int (*delete) (event_t *this, int fd, event_type_t type);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (event_t *this);

    /**
     * @brief exception handle
     */
    void (*exception_handle) (event_t *this, exception_type_t type, void (*handler) (void *), void *arg);
};

/**
 * @brief create socket event instance 
 */
event_t *event_create(event_mode_t mode, int timeout);

#endif /* __SOCKET_EVENT__ */
