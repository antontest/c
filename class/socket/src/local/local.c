#include <local.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils/utils.h>
#include <host.h>
#include <arpa/inet.h>
#include <sys/un.h>

typedef enum server_type_t server_type_t;
enum server_type_t {
    TCP_CLIENT = 1 << 1,
    TCP_SERVER = 3 << 1,
    UDP_CLIENT = 1 << 2,
    UDP_SERVER = 3 << 2
};

typedef struct private_local_socket_t private_local_socket_t;
struct private_local_socket_t {
    /**
     * @brief public interface
     */
    local_socket_t public;

    /**
     * @brief socket handler
     */
    int fd;

    /**
     * @brief tcp accept fd
     */
    int accepted_fd;

    /**
     * @brief local socket type, SOCK_STREAM or SOCK_DGRAM
     */
    int type;

    /**
     * @brief socket address package
     */
    struct sockaddr_un *addr;

    /**
     * @brief server_type
     */
    server_type_t s_type;
};
#define local_fd    this->fd
#define accept_fd   this->accepted_fd
#define local_addr  this->addr
#define local_type  this->type
#define server_type this->s_type

METHOD(local_socket_t, socket_, int, private_local_socket_t *this, int type)
{
    local_type = type;
    local_fd = socket(AF_UNIX, type, 0);
    switch (type) {
        case SOCK_STREAM:
            server_type = TCP_CLIENT;
            break;
        default:
            server_type = UDP_CLIENT;
            break;
    }
    return local_fd;
}

METHOD(local_socket_t, init_addr, struct sockaddr *, private_local_socket_t *this, const char *path)
{
    if (!path) return NULL;
    if (local_addr) return (struct sockaddr *)local_addr;

    if (!local_addr) local_addr  = (struct sockaddr_un *)malloc(sizeof(struct sockaddr_un));    
    if (!local_addr) return NULL;
    memset(local_addr, 0, sizeof(struct sockaddr_un));
    local_addr->sun_family = AF_UNIX;
    strncpy(local_addr->sun_path, path, sizeof(local_addr->sun_path));
    return (struct sockaddr *)local_addr;
}

METHOD(local_socket_t, bind_, int, private_local_socket_t *this)
{
    if (!local_addr) return -1;
    if (!access(local_addr->sun_path, F_OK)) return -1;
    switch (server_type) {
        case TCP_CLIENT:
            server_type = TCP_SERVER;
            break;
        default:
            server_type = UDP_SERVER;
            break;
    }
    return bind(local_fd, (struct sockaddr *)local_addr, sizeof(struct sockaddr));
}

METHOD(local_socket_t, listen_, int, private_local_socket_t *this, int backlog)
{
    if (server_type != TCP_SERVER) return -1;
    return listen(local_fd, backlog);
}

METHOD(local_socket_t, accept_, int, private_local_socket_t *this, struct sockaddr *addr)
{
    if (server_type != TCP_SERVER) return -1;
    socklen_t len = sizeof(struct sockaddr);
    accept_fd = accept(local_fd, addr, &len);
    return accept_fd;
}

METHOD(local_socket_t, connect_, int, private_local_socket_t *this)
{
    if (server_type != TCP_CLIENT) return -1;
    return connect(local_fd, (struct sockaddr *)local_addr, sizeof(struct sockaddr));
}

METHOD(local_socket_t, send_, int, private_local_socket_t *this, void *buf, int size, int flags)
{
    int send_fd = -1;
    
    switch (server_type) {
        case TCP_CLIENT:
            send_fd = local_fd;
            break;
        case TCP_SERVER:
            send_fd = accept_fd;
            break;
        default:
            break;
    }

    return send(send_fd, buf, size, flags);
}

METHOD(local_socket_t, sendto_, int, private_local_socket_t *this, void *buf, int size, int flags, struct sockaddr *addr)
{
    return sendto(local_fd, buf, size, flags, addr, sizeof(struct sockaddr));
}

METHOD(local_socket_t, recv_, int, private_local_socket_t *this, void *buf, int size, int flags)
{
    int recv_fd = -1;

    switch (server_type) {
        case TCP_CLIENT:
            recv_fd = local_fd;
            break;
        case TCP_SERVER:
            recv_fd = accept_fd;
            break;
        default:
            break;
    }

    return recv(recv_fd, buf, size, flags);
}

METHOD(local_socket_t, recvfrom_, int, private_local_socket_t *this, void *buf, int size, int flags, struct sockaddr *addr)
{
    socklen_t len = sizeof(struct sockaddr);
    return recvfrom(local_fd, buf, size, flags, addr, &len);
}

METHOD(local_socket_t, close_, int, private_local_socket_t *this)
{
    switch (server_type) {
        case TCP_CLIENT:
        case UDP_CLIENT:
        case UDP_SERVER:
            break;
        case TCP_SERVER:
            close(accept_fd);
            break;
        default:
            return -1;
            break;
    }
    if (local_addr && local_addr->sun_path != NULL) {
        unlink(local_addr->sun_path);
    }
    return close(local_fd);
}

METHOD(local_socket_t, shutdown_, int, private_local_socket_t *this, int how)
{
    switch (server_type) {
        case TCP_CLIENT:
            return shutdown(local_fd, how);
            break;
        case TCP_SERVER:
            return shutdown(accept_fd, how);
            break;
        case UDP_CLIENT:
        case UDP_SERVER:
            return shutdown(local_fd, how);
            break;
        default:
            return -1;
            break;
    }
}

METHOD(local_socket_t, destroy_, void, private_local_socket_t *this)
{
    close_(this);
    if (local_addr) free(local_addr);
    free(this);
}

METHOD(local_socket_t, get_fd, int, private_local_socket_t *this)
{
    return local_fd;
}

METHOD(local_socket_t, get_accepted_fd, int, private_local_socket_t *this)
{
    return accept_fd;
}

local_socket_t *create_local_socket()
{
    private_local_socket_t *this;

    INIT(this, 
        .public = {
            .socket    = _socket_,
            .init_addr = _init_addr,
            .bind      = _bind_,
            .listen    = _listen_,
            .accept    = _accept_,
            .connect   = _connect_,
            .send      = _send_,
            .sendto    = _sendto_,
            .recv      = _recv_,
            .recvfrom  = _recvfrom_,
            .close     = _close_,
            .shutdown  = _shutdown_,
            .destroy   = _destroy_,
            
            .get_fd          = _get_fd,
            .get_accepted_fd = _get_accepted_fd,
        },
        .type        = -1,
        .addr        = NULL,
        .s_type      = 0,
        .fd          = -1,
        .accepted_fd = -1,
    );

    return &this->public;
}
