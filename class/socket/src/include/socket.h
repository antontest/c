#ifndef __SOCKET_H__
#define __SOCKET_H__

typedef struct socket socket_t;
struct socket {
    /**
     * @brief create a socket
     *
     * @param 
     */
    int (*create)(socket_t *this);
};

#endif /* __SOCKET_H__ */
