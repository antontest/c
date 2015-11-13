#include <socket_base.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <host.h>
#include <utils/utils.h>

typedef struct private_socket_base_t private_socket_base_t;
struct private_socket_base_t {
    /**
     * @brief public interface
     */
    socket_base_t public;

    /**
     * @brief socket instance descriptor
     */
    int fd;

    /**
     * @brief accept socket descriptor in tcp
     */
    int accept_fd;

    /**
     * @brief AF_INET, AF_INET6
     */
    int family;

    /**
     * @brief socket type, like SOCK_STREAM
     */
    int type;

    /**
     * @brief socket address information with IP address and port
     */
    host_t *host;

    /**
     * @brief flag describ whether server or client
     */
    int ser_or_cli_flag;
};

METHOD(socket_base_t, socket_, int, private_socket_base_t *this, int family, int type, int protocol)
{
    this->fd     = socket(family, type, protocol);
    this->family = family;
    this->type   = type;
    return this->fd;
}

METHOD(socket_base_t, init_addr, struct sockaddr *, private_socket_base_t *this, int family, char *ip, unsigned int port)
{
    this->host = host_create_from_string_and_family(ip, family, port);
    if (!this->host) return NULL;
    return this->host->get_sockaddr(this->host);
}

METHOD(socket_base_t, bind_, int, private_socket_base_t *this, char *ip, unsigned int port)
{
    this->ser_or_cli_flag = 1;
    this->host = host_create_from_string_and_family(ip, this->family, port);
    if (!this->host) return -1;

    return bind(this->fd, this->host->get_sockaddr(this->host), *(int *)this->host->get_sockaddr_len(this->host));
}

METHOD(socket_base_t, listen_, int, private_socket_base_t *this, int backlog)
{
    return listen(this->fd, backlog);
}

METHOD(socket_base_t, accept_, int, private_socket_base_t *this, struct sockaddr *addr)
{
    socklen_t len;
    len = addr == NULL ? 0 : sizeof(struct sockaddr);
    this->accept_fd = accept(this->fd, (struct sockaddr *)addr, &len);
    return this->accept_fd;
}

METHOD(socket_base_t, connect_, int, private_socket_base_t *this, char *ip, unsigned int port)
{
    this->host = host_create_from_string_and_family(ip, this->family, port);
    if (!this->host) return -1;

    return connect(this->fd, this->host->get_sockaddr(this->host), sizeof(struct sockaddr));
}

METHOD(socket_base_t, send_, int, private_socket_base_t *this, void *buf, int size, int flags)
{
    if (this->ser_or_cli_flag) return send(this->accept_fd, buf, size, flags);
    return send(this->fd, buf, size, flags);
}

METHOD(socket_base_t, sendto_, int, private_socket_base_t *this, void *buf, int size, int flags, struct sockaddr *dest_addr, int addrlen)
{
    if (this->ser_or_cli_flag) return sendto(this->accept_fd, buf, size, flags, dest_addr, (socklen_t)addrlen);
    return sendto(this->fd, buf, size, flags, dest_addr, (socklen_t)addrlen);
}

METHOD(socket_base_t, recv_, int, private_socket_base_t *this, void *buf, int size, int flags)
{
    if (this->ser_or_cli_flag) return recv(this->accept_fd, buf, size, flags);
    return recv(this->fd, buf, size, flags);
}

METHOD(socket_base_t, recvfrom_, int, private_socket_base_t *this, void *buf, int size, int flags, struct sockaddr *src_addr, int *addrlen)
{
    return recvfrom(this->fd, buf, size, flags, src_addr, (socklen_t *)addrlen);
}

METHOD(socket_base_t, close_, int, private_socket_base_t *this, int fd)
{
    return close(this->fd);
}

METHOD(socket_base_t, shutdown_, int, private_socket_base_t *this, int fd, int how)
{
    return shutdown(fd, how);
}

METHOD(socket_base_t, destroy_, void, private_socket_base_t *this)
{
    if (this->fd > 0) close(this->fd);
    if (this->accept_fd > 0) close(this->accept_fd);
    if (this->host != NULL) this->host->destroy(this->host);
    free(this);
}

METHOD(socket_base_t, get_fd, int, private_socket_base_t *this)
{
    if (this->type == SOCK_DGRAM) return this->fd;
    return this->accept_fd;
}

socket_base_t *create_socket_base()
{
    private_socket_base_t *this;

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
            
            .get_fd    = _get_fd,
        },
        .fd              = -1,
        .accept_fd       = -1,
        .host            = NULL,
        .ser_or_cli_flag = 0,
    );

    return &this->public;
}
