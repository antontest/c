#ifndef __UDP_H__
#define __UDP_H__

typedef struct udp_t udp_t;
struct udp_t {
    /**
     * @brief server listen
     *
     * @param ip   [in] ip address listening on, can be NULL;
     * @param port [in] port listening on, must be more than 0;
     * @return     socket fd, if succ; -1, if failed;
     */
    int (*listen) (udp_t *this, int family, char *ip, int port);

    /**
     * @brief connect to server 
     *
     * @param ip   [in] ip address of server;
     * @param port [in] port of server listening on;
     * @return     socket fd, if succ; -1, if failed;
     */
    int (*connect) (udp_t *this, int family, char *ip, int port);

    /**
     * @brief tcp server accept 
     *
     * @return accept fd, if succ; -1, if failed
     */
    int (*accept) (udp_t *this);

    /**
     * @brief send message
     *
     * @param buf  [in] message buffer
     * @param size [in] size of message
     * @return     count of message sended, if succ; -1, if failed;
     */
    int (*send) (udp_t *this, void *buf, int size);

    /**
     * @brief recv message
     *
     * @param buf  [out] message buffer
     * @param size [in]  size of message
     * @return     count of message recved, if succ; -1, if failed;
     */
    int (*recv) (udp_t *this, void *buf, int size);
    
    /**
     * @brief close tcp connection
     */
    int (*close) (udp_t *this);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (udp_t *this);
};

/**
 * @brief create udp instance
 */
udp_t *udp_create();

#endif /* __UDP_H__ */
