#include <socket.h>
#include <chunk.h>

/* for struct in6_pktinfo */
#define _GNU_SOURCE
#ifdef __sun
#define _XPG4_2
#define __EXTENSIONS__
#endif
/* make sure to use the proper defs on Mac OS X */
#define __APPLE_USE_RFC_3542

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/if.h>

#include <signal.h>
#include <thread/thread.h>
#include <host.h>

/* Maximum size of a packet */
#define MAX_PACKET 10000

/* these are not defined on some platforms */
#ifndef SOL_IP
#define SOL_IP IPPROTO_IP
#endif
#ifndef SOL_IPV6
#define SOL_IPV6 IPPROTO_IPV6
#endif
#ifndef IPV6_TCLASS
#define IPV6_TCLASS 67
#endif

/* IPV6_RECVPKTINFO is defined in RFC 3542 which obsoletes RFC 2292 that
 * previously defined IPV6_PKTINFO */
#ifndef IPV6_RECVPKTINFO
#define IPV6_RECVPKTINFO IPV6_PKTINFO
#endif

#ifndef IN6ADDR_ANY_INIT
#define IN6ADDR_ANY_INIT {{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}}
#endif

typedef enum socket_state_t socket_state_t;
enum socket_state_t {
    SOCKET_CLOSED = 0,
    SOCKET_STARTING,
    SOCKET_CONNECTING,
    SOCKET_CONNECTED,
    SOCKET_CONNECT_ERROR,
    SOCKET_SENDING,
    SOCKET_SENDED,
    SOCKET_SEND_ERROR,
    SOCKET_RECEIVING,
    SOCKET_RECEIVED,
    SOCKET_RECEIVE_ERROR, 
};

typedef struct private_socket_t private_socket_t;

/**
 * Private data of an socket_t object
 */
struct private_socket_t {

    /**
     * public functions
     */
    socket_t public;

    /**
     * socket fd
     */
    int fd;

    /**
     * socket fd
     */
    int accept_fd;

    /**
     * Configured addr
     */
    host_t *host;

    /**
     * socket type;
     */
    int type;

    /**
     * socket state;
     */
    int state;

    /**
     * Maximum packet size to receive
     */
    int max_packet;

    /**
     * TRUE if the source address should be set on outbound packets
     */
    bool set_source;
};

METHOD(socket_t, receiver, int,
        private_socket_t *this, void *buf, int size, int timeout)
{
    int max_fd = 0;
    int select_fd = 0;
    int can_read_bytes = 0;
    struct timeval tv = {0};
    void *timeout_ptr = NULL;
    fd_set fds;

    FD_ZERO(&fds);
    if (this->fd != -1) {
        FD_SET(this->fd, &fds);
        max_fd = max(max_fd, this->fd);
    }
    if (this->accept_fd != -1) {
        FD_SET(this->accept_fd, &fds);
        max_fd = max(max_fd, this->accept_fd);
    }
    if (max_fd < 1) {
        return -1;
    }
    max_fd++;
    this->state = SOCKET_RECEIVING;

    if (timeout > 0) {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = timeout % 1000;
        timeout_ptr = &tv;
    }
    if (select(max_fd, &fds, NULL, NULL, timeout_ptr) < 0) {
        this->state = SOCKET_RECEIVE_ERROR;
        printf("select failed\n");
        return -1;
    }

    if (FD_ISSET(this->fd, &fds)) {
        select_fd = this->fd;
    }
    if (!select_fd && FD_ISSET(this->accept_fd, &fds)) {
        select_fd = this->accept_fd;
    }
    if (select_fd <= 0) {
        return -1;
    }

    ioctl(select_fd, FIONREAD, &can_read_bytes);
    if (can_read_bytes > 0) {
        size = can_read_bytes < size ? can_read_bytes : size;
    }
    switch (this->type) {
        case SOCK_DGRAM:
            this->state = SOCKET_RECEIVED;
            return recvfrom(this->fd, buf, size, 0, NULL, NULL);
        default:
            this->state = SOCKET_RECEIVED;
            return recv(this->accept_fd, buf, size, 0);
    }
}

METHOD(socket_t, sender, int,
        private_socket_t *this, void *buf, int size)
{
    int send_cnt = 0;

    this->state = SOCKET_SENDING;
    switch (this->type) {
        case SOCK_DGRAM:
            send_cnt = sendto(this->fd, buf, size, 0, (struct sockaddr *)this->host->get_sockaddr(this->host), (socklen_t)sizeof(struct sockaddr));
        default:
            send_cnt = send(this->accept_fd, buf, size, 0);
    }
    this->state = SOCKET_SENDED;

    return send_cnt;
}

METHOD(socket_t, listener, int, private_socket_t *this, int family, int type, int prototype, char *ip, unsigned short port)
{
    int on = 1;

    this->state = SOCKET_CLOSED;
    this->host = host_create_from_string_and_family(ip, family, port);
    
    if (this->host == NULL) {
        fprintf(stderr, "host create failed\n");
        return -1;
    }

    if ((this->fd = socket(family, type, prototype)) < 0) {
        fprintf(stderr, "could not open socket: %s", strerror(errno));
        return -1;
    }

    if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) < 0)
    {        
        fprintf(stderr, "unable to set SO_REUSEADDR on socket: %s", strerror(errno));
        close(this->fd);
        return -1;
    }

    if (bind(this->fd, (struct sockaddr *)this->host->get_sockaddr(this->host), (socklen_t)sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "unable to bind socket: %s\n", strerror(errno));
        close(this->fd);
        return -1;
    }
    this->type = type;

    if (type == SOCK_DGRAM) {
        this->state = SOCKET_STARTING;
        return this->fd;
    }

    if (listen(this->fd, 5) < 0) {
        fprintf(stderr, "unable to listen socket: %s", strerror(errno));
        close(this->fd);
        return -1;
    }
    this->state = SOCKET_STARTING;

    return this->fd;
}

METHOD(socket_t, connecter, int, private_socket_t *this, int family, int type, int prototype, char *ip, unsigned short port)
{
    int on = 1;

    this->state = SOCKET_STARTING;
    this->host = host_create_from_string_and_family(ip, family, port);
    
    if (this->host == NULL) {
        fprintf(stderr, "host create failed\n");
        return -1;
    }

    if ((this->fd = socket(family, type, prototype)) < 0) {
        fprintf(stderr, "could not open socket: %s", strerror(errno));
        return -1;
    }

    if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) < 0)
    {        
        fprintf(stderr, "unable to set SO_REUSEADDR on socket: %s", strerror(errno));
        close(this->fd);
        return -1;
    }
    this->type = type;

    if (type == SOCK_DGRAM) {
        return this->fd;
    }

    this->state = SOCKET_CONNECTING;
    if (connect(this->fd, (struct sockaddr *)this->host->get_sockaddr(this->host), (socklen_t)sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "unable to connect to server: %s", strerror(errno));
        this->state = SOCKET_CONNECT_ERROR;
        close(this->fd);
        return -1;
    }
    this->state = SOCKET_CONNECTED;

    return this->fd;
}

METHOD(socket_t, accepter, int, private_socket_t *this)
{
    if ((this->accept_fd = accept(this->fd, NULL, 0)) < 0) {
        fprintf(stderr, "accept failed: %s\n", strerror(errno));
        close(this->fd);
        return -1;
    }
    this->state = SOCKET_CONNECTED;

    return this->accept_fd;
}


METHOD(socket_t, get_port, unsigned short,
        private_socket_t *this)
{
    return this->host->get_port(this->host);
}

METHOD(socket_t, get_family, unsigned short, private_socket_t *this)
{
    return this->host->get_family(this->host);
}

METHOD(socket_t, get_sockaddr, struct sockaddr *, private_socket_t *this)
{
    return this->host->get_sockaddr(this->host);
}

METHOD(socket_t, get_type, int, private_socket_t *this)
{
    return this->type;
}

METHOD(socket_t, get_sockfd, int, private_socket_t *this)
{
    switch (this->type) {
        case SOCK_DGRAM:
            return this->fd;
        default:
            return this->accept_fd;
    }
}

METHOD(socket_t, get_state, int, private_socket_t *this)
{
    int can_read_bytes = 0;

    if (this->state <= SOCKET_STARTING) {
        return this->state;
    }

    if (this->type == SOCK_DGRAM) {
        ioctl(this->fd, FIONREAD, &can_read_bytes);
    } else {
        ioctl(this->accept_fd, FIONREAD, &can_read_bytes);
    }

    if (can_read_bytes <= 0) {
        this->state = SOCKET_CLOSED;
    }
    return this->state;
}

METHOD(socket_t, destroy, void, private_socket_t *this)
{
    if (this->accept_fd != -1)
    {
        close(this->accept_fd);
    }

    if (this->fd != -1)
    {
        close(this->fd);
    }

    free(this->host);
    free(this);
}


/*
 * See header for description
 */
socket_t *socket_create()
{
    private_socket_t *this;

    INIT(this,
            .public = {
            .listen       = _listener,
            .accept       = _accepter,
            .connect      = _connecter,
            .send         = _sender,
            .receive      = _receiver,
            .get_port     = _get_port,
            .get_family   = _get_family,
            .get_sockaddr = _get_sockaddr,
            .get_type     = _get_type,
            .get_sockfd   = _get_sockfd,
            .get_state    = _get_state,
            .destroy      = _destroy,
            },
            .host      = NULL,
            .state     = SOCKET_CLOSED,
            .fd        = -1,
            .accept_fd = -1,
    );

    return &this->public;
}
