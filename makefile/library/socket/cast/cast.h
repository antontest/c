#ifndef __CAST_H__
#define __CAST_H__

typedef struct cast_t cast_t;
struct cast_t {
    /**
     * @brief send broadcast message
     * @param buf  message buffer 
     * @param size size of message buffer
     * @return  count of sending
     */
    int (*send) (cast_t *this, void *buf, int size);

    /**
     * @brief recv broadcast message
     * @param buf  message buffer 
     * @param size size of message buffer
     * @return  count of recving
     */
    int (*recv) (cast_t *this, void *buf, int size);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (cast_t *this);
};

/**
 * @brief create_broadcast
 *
 * @param ip   255.255.255.255
 * @param port port
 */
cast_t *create_broadcast(const char *ip, int port);

/**
 * @brief create_multicast 
 *
 * @param ip   224.0.0.0 - 239.255.255.255
 * @param port port
 */
cast_t *create_multicast(const char *ip, int port);

#endif /* __CAST_H__ */
