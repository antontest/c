#include <tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <utils.h>
#include <host.h>

typedef struct private_tcp_t private_tcp_t;
struct private_tcp_t {
    /**
     * @brief public interface
     */
    tcp_t public;

    /**
     * @brief socket handler fd
     */
    int fd;

    /**
     * @brief tcp server accept fd
     */
    int accept_fd;

    /**
     * @brief socket host
     */
    host_t *host;
};
#define tcp_fd        this->fd
#define tcp_host      this->host
#define tcp_accept_fd this->accept_fd

METHOD(tcp_t, listen_, int, private_tcp_t *this, int family, char *ip, int port)
{
    int ret = 0;

    /**
     * create socket
     */
    if (tcp_fd > 0) close(tcp_fd);
    tcp_fd = socket(family, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        perror("socket()");
        return -1;
    }

    /**
     * create host
     */
    if (tcp_host) tcp_host->destroy(tcp_host);
    tcp_host = host_create_from_string_and_family(ip ? ip : "%any", family, port);
    if (!tcp_host) {
        printf("create host failed\n");
        return -1;
    }

    /**
     * socket bind
     */
    ret = bind(tcp_fd, tcp_host->get_sockaddr(tcp_host), sizeof(struct sockaddr));
    if (ret < 0) {
        perror("bind()");
        return -1;
    }

    /**
     * socket listen 
     */
    ret = listen(tcp_fd, 5);
    if (ret < 0) {
        perror("listen()");
        return -1;
    }

    return tcp_fd;
}

METHOD(tcp_t, connect_, int, private_tcp_t *this, int family, char *ip, int port)
{
    int ret = 0;

    /**
     * create socket
     */
    if (tcp_accept_fd <= 0) tcp_accept_fd = socket(family, SOCK_STREAM, 0);
    if (tcp_accept_fd <= 0) {
        perror("socket()");
        return -1;
    }

    /**
     * create host
     */
    if (tcp_host) {
        if (tcp_host->get_family(tcp_host) != family || tcp_host->get_port(tcp_host) != port || strcmp(tcp_host->get_ip(tcp_host, NULL, 0), ip)) {
            tcp_host->destroy(tcp_host);
            tcp_host = NULL;
    }
    }
    if (!tcp_host) tcp_host = host_create_from_string_and_family(ip ? ip : "%any", family, port);
    if (!tcp_host) {
        printf("create host failed\n");
        return -1;
    }

    /**
     * connect to server
     */
    ret = connect(tcp_accept_fd, tcp_host->get_sockaddr(tcp_host), sizeof(struct sockaddr));
    if (ret < 0) {
        perror("connect()");
        return -1;
    }

    return tcp_accept_fd;
}

METHOD(tcp_t, accept_, int, private_tcp_t *this)
{
    tcp_accept_fd = accept(tcp_fd, NULL, 0);
    return tcp_accept_fd;
}

METHOD(tcp_t, send_, int, private_tcp_t *this, void *buf, int size)
{
    return send(tcp_accept_fd, buf, size, 0);   
}

METHOD(tcp_t, recv_, int, private_tcp_t *this, void *buf, int size)
{
    return recv(tcp_accept_fd, buf, size, 0);
}

METHOD(tcp_t, close_, int, private_tcp_t *this)
{
    return close(tcp_accept_fd);
}

METHOD(tcp_t, destroy_, void, private_tcp_t *this)
{
    if (tcp_fd) close(tcp_fd);
    if (tcp_accept_fd) close(tcp_accept_fd);
    if (tcp_host) tcp_host->destroy(tcp_host);
    free(this);
}

tcp_t *tcp_create(int family)
{
    private_tcp_t *this;

    INIT(this, 
        .public = {
            .listen  = _listen_,
            .connect = _connect_,
            .accept  = _accept_,
            .send    = _send_,
            .recv    = _recv_,
            .close   = _close_,
            .destroy = _destroy_
        },
        .fd   = -1,
        .host = NULL,
    );

    return &this->public;
}
