#ifndef __TCP_H__
#define __TCP_H__

typedef struct tcp_t tcp_t;
struct tcp_t {
    /**
     * @brief server listen
     *
     * @param ip   [in] ip address listening on, can be NULL;
     * @param port [in] port listening on, must be more than 0;
     * @return     socket fd, if succ; -1, if failed;
     */
    int (*listen) (tcp_t *this, int family, char *ip, int port);

    /**
     * @brief connect to server 
     *
     * @param ip   [in] ip address of server;
     * @param port [in] port of server listening on;
     * @return     socket fd, if succ; -1, if failed;
     */
    int (*connect) (tcp_t *this, int family, char *ip, int port);
    int (*connect_tm) (tcp_t *this, int family, char *ip, int port, int timeout_ms);

    /**
     * @brief tcp server accept 
     *
     * @return accept fd, if succ; -1, if failed
     */
    int (*accept) (tcp_t *this);

    /**
     * @brief send message
     *
     * @param buf  [in] message buffer
     * @param size [in] size of message
     * @return     count of message sended, if succ; -1, if failed;
     */
    int (*send) (tcp_t *this, void *buf, int size);

    /**
     * @brief recv message
     *
     * @param buf  [out] message buffer
     * @param size [in]  size of message
     * @return     count of message recved, if succ; -1, if failed;
     */
    int (*recv) (tcp_t *this, void *buf, int size);
    int (*recv_tm) (tcp_t *this, void *buf, int size, int timeout_ms);
    
    /**
     * @brief close tcp connection
     */
    int (*close) (tcp_t *this);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (tcp_t *this);
};

/**
 * @brief create tcp instance
 */
tcp_t *tcp_create();

#endif /* __TCP_H__ */
