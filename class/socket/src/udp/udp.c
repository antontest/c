#include <udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <utils/utils.h>
#include <host.h>

typedef struct private_udp_t private_udp_t;
struct private_udp_t {
    /**
     * @brief public interface
     */
    udp_t public;

    /**
     * @brief socket fd
     */
    int fd;

    /**
     * @brief socket host
     */
    host_t *host;
};
#define udp_fd   this->fd
#define udp_host this->host

METHOD(udp_t, listen_, int, private_udp_t *this, int family, char *ip, int port)
{
    int ret = 0;

    udp_fd = socket(family, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("socket()");
        return -1;
    }

    udp_host = host_create_from_string_and_family(ip ? ip : "%any", family, port);
    if (!udp_host) {
        printf("create host failed\n");
        return -1;
    }

    ret = bind(udp_fd, udp_host->get_sockaddr(udp_host), sizeof(struct sockaddr));
    if (ret < 0) {
        perror("bind()");
        return -1;
    }
    
    return udp_fd;
}

METHOD(udp_t, connect_, int, private_udp_t *this, int family, char *ip, int port)
{
    udp_fd = socket(family, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("socket()");
        return -1;
    }

    udp_host = host_create_from_string_and_family(ip ? ip : "%any", family, port);
    if (!udp_host) {
        printf("create host failed\n");
        return -1;
    }

    return udp_fd;
}

udp_t *udp_create()
{
    private_udp_t *this;

    INIT(this, 
        .public = {
        },
        .fd   = -1,
        .host = NULL,
    );

    return &this->public;
}
