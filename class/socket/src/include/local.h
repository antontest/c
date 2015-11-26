#ifndef __LOCAL_H__
#define __LOCAL_H__
#include <sys/types.h>
#include <sys/socket.h>

typedef struct local_socket_t local_socket_t;
struct local_socket_t {
    int (*socket) (local_socket_t *this, int type);
    struct sockaddr *(*init_addr) (local_socket_t *this, const char *path);
    int (*bind) (local_socket_t *this);
    int (*listen) (local_socket_t *this, int backlog);
    int (*accept) (local_socket_t *this, struct sockaddr *addr);
    int (*connect) (local_socket_t *this);
    int (*send) (local_socket_t *this, void *buf, int size, int flags);
    int (*sendto) (local_socket_t *this, void *buf, int size, int flags, struct sockaddr *addr);
    int (*recv) (local_socket_t *this, void *buf, int size, int flags);
    int (*recvfrom) (local_socket_t *this, void *buf, int size, int flags, struct sockaddr *addr);
    int (*close) (local_socket_t *this);
    int (*shutdown) (local_socket_t *this, int how);
    void (*destroy) (local_socket_t *this);
    int (*get_fd) (local_socket_t *this);
    int (*get_accepted_fd) (local_socket_t *this);
};

/**
 * @brief create local socket instance 
 */
local_socket_t *create_local_socket();

#endif /* __LOCAL_H__ */
