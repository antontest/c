#include <socket_base.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <stdarg.h>
#include <host.h>
#include <utils/utils.h>

#define DFT_SND_BUFF_SIZE (1024)
#define DFT_RCV_BUFF_SIZE (1024)

typedef enum socket_type_t socket_type_t;
enum socket_type_t {
    SOCKET_SERVER = 1 << 1,
    SOCKET_CLIENT = 1 << 2
};

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
    socket_type_t sock_type;

    /**
     * @brief send buffer
     */
    char *snd_buffer;

    /**
     * @brief send buffer size
     */
    unsigned int snd_buff_size;

    /**
     * @brief recv buffer
     */
    char *rcv_buffer;

    /**
     * @brief recv buffer size
     */
    unsigned int rcv_buff_size;
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
    this->sock_type = SOCKET_SERVER;
    if (!this->host)
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
    len = (addr == NULL) ? 0 : sizeof(struct sockaddr);
    this->accept_fd = accept(this->fd, (struct sockaddr *)addr, &len);
    return this->accept_fd;
}

METHOD(socket_base_t, connect_, int, private_socket_base_t *this, char *ip, unsigned int port)
{
    if (!this->host)
        this->host = host_create_from_string_and_family(ip, this->family, port);
    if (!this->host) return -1;

    return connect(this->fd, this->host->get_sockaddr(this->host), sizeof(struct sockaddr));
}

METHOD(socket_base_t, send_, int, private_socket_base_t *this, void *buf, int size, int flags)
{
    if (this->sock_type == SOCKET_SERVER) {
        if (this->fd == this->accept_fd) return -1;
        return send(this->accept_fd, buf, size, flags);
    }
    return send(this->fd, buf, size, flags);
}

METHOD(socket_base_t, vsend_, int, private_socket_base_t *this, int flags, const char *fmt, ...)
{
    va_list arg;
    int size = 0;

    if (!this->snd_buffer) this->snd_buffer = (char *)malloc(this->snd_buff_size);
    va_start(arg, fmt);
    size = vsnprintf(this->snd_buffer, this->snd_buff_size - 1, fmt, arg);
    this->snd_buffer[size + 1] = '\0';
    va_end(arg);

    if (this->sock_type == SOCKET_SERVER) {
        if (this->fd == this->accept_fd) return -1;
        return send(this->accept_fd, this->snd_buffer, size, flags);
    }
    return send(this->fd, this->snd_buffer, size, flags);
}

METHOD(socket_base_t, sendto_, int, private_socket_base_t *this, void *buf, int size, int flags, struct sockaddr *dest_addr, int addrlen)
{
    if (this->sock_type == SOCKET_SERVER) return sendto(this->accept_fd, buf, size, flags, dest_addr, (socklen_t)addrlen);
    return sendto(this->fd, buf, size, flags, dest_addr, (socklen_t)addrlen);
}

METHOD(socket_base_t, vsendto_, int, private_socket_base_t *this, int flags, struct sockaddr *dest_addr, int addrlen, const char *fmt, ...)
{
    va_list arg;
    int size = 0;

    if (!this->snd_buffer) this->snd_buffer = (char *)malloc(this->snd_buff_size);
    va_start(arg, fmt);
    size = vsnprintf(this->snd_buffer, this->snd_buff_size - 1, fmt, arg);
    this->snd_buffer[size + 1] = '\0';
    va_end(arg);

    if (this->sock_type == SOCKET_SERVER) return sendto(this->accept_fd, this->snd_buffer, size, flags, dest_addr, (socklen_t)addrlen);
    return sendto(this->fd, this->snd_buffer, size, flags, dest_addr, (socklen_t)addrlen);
}

METHOD(socket_base_t, recv_, int, private_socket_base_t *this, void *buf, int size, int flags)
{
    if (this->sock_type == SOCKET_SERVER) {
        if (this->fd == this->accept_fd) return -1;
        return recv(this->accept_fd, buf, size, flags);
    }
    return recv(this->fd, buf, size, flags);
}

METHOD(socket_base_t, vrecv_, char *, private_socket_base_t *this, int flags)
{
    if (!this->rcv_buffer) this->rcv_buffer = (char *)malloc(this->rcv_buff_size);

    if (this->sock_type == SOCKET_SERVER) {
        if (this->fd == this->accept_fd) return NULL;
        recv(this->accept_fd, this->rcv_buffer, this->rcv_buff_size, flags);
    }else recv(this->fd, this->rcv_buffer, this->rcv_buff_size, flags);

    return this->rcv_buffer;
}

METHOD(socket_base_t, recvfrom_, int, private_socket_base_t *this, void *buf, int size, int flags, struct sockaddr *src_addr, int *addrlen)
{
    return recvfrom(this->fd, buf, size, flags, src_addr, (socklen_t *)addrlen);
}

METHOD(socket_base_t, vrecvfrom_, char *, private_socket_base_t *this, int flags, struct sockaddr *src_addr, int *addrlen)
{
    if (!this->rcv_buffer) this->rcv_buffer = (char *)malloc(this->rcv_buff_size);
    recvfrom(this->fd, this->rcv_buffer, this->rcv_buff_size, flags, src_addr, (socklen_t *)addrlen);

    return this->rcv_buffer;
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
    if (this->fd > 0)             close(this->fd);
    if (this->accept_fd > 0)      close(this->accept_fd);
    if (this->host != NULL)       this->host->destroy(this->host);
    if (this->snd_buffer != NULL) free(this->snd_buffer);
    if (this->rcv_buffer != NULL) free(this->rcv_buffer);
    free(this);
}

METHOD(socket_base_t, get_fd_, int, private_socket_base_t *this)
{
    if (this->type == SOCK_DGRAM) return this->fd;
    return this->accept_fd;
}

METHOD(socket_base_t, set_send_buffer_size_, void, private_socket_base_t *this, unsigned int size)
{
    if (!this->snd_buffer) {
        this->snd_buffer = (char *)malloc(size);
        if (!this->snd_buffer) this->snd_buff_size = 0;
        else this->snd_buff_size = size;
    } else {
        char *p = realloc(this->snd_buffer, size);
        if (p != NULL) {
            this->snd_buffer    = p;
            this->snd_buff_size = size;
        }
    }
}

METHOD(socket_base_t, set_recv_buffer_size_, void, private_socket_base_t *this, unsigned int size)
{
    if (!this->rcv_buffer) {
        this->rcv_buffer= (char *)malloc(size);
        if (!this->rcv_buffer) this->rcv_buff_size = 0;
        else this->rcv_buff_size = size;
    } else {
        char *p = realloc(this->rcv_buffer, size);
        if (p != NULL) {
            this->rcv_buffer    = p;
            this->rcv_buff_size = size;
        }
    }
}

socket_base_t *create_socket_base()
{
    private_socket_base_t *this;

    INIT(this,
        .public = {
            .init_addr = _init_addr,
            .socket    = _socket_,
            .bind      = _bind_,
            .listen    = _listen_,
            .accept    = _accept_,
            .connect   = _connect_,
            .close     = _close_,
            .shutdown  = _shutdown_,
            .destroy   = _destroy_,

            .send      = _send_,
            .vsend     = _vsend_,
            .sendto    = _sendto_,
            .vsendto   = _vsendto_,

            .recv      = _recv_,
            .vrecv     = _vrecv_,
            .recvfrom  = _recvfrom_,
            .vrecvfrom = _vrecvfrom_,
            
            .get_fd    = _get_fd_,

            .set_send_buffer_size = _set_send_buffer_size_,
            .set_recv_buffer_size = _set_recv_buffer_size_,
        },
        .fd              = -1,
        .accept_fd       = -1,
        .host            = NULL,
        .sock_type       = SOCKET_SERVER,
        .snd_buffer      = NULL,
        .snd_buff_size   = DFT_SND_BUFF_SIZE,
        .rcv_buffer      = NULL,
        .rcv_buff_size   = DFT_RCV_BUFF_SIZE,
    );

    return &this->public;
}
