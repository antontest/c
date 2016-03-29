#include <cast.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <utils/utils.h>
#include <base/socket_base.h>
#include <property/property.h>

#define BROADCAST_IP "255.255.255.255"
typedef struct private_cast_t private_cast_t;
struct private_cast_t {
    /**
     * @brief public interface
     */
    cast_t public;

    /**
     * @brief ip address
     */
    char *ip;

    /**
     * @brief cast port
     */
    int port;

    /**
     * @brief to addr
     */
    struct sockaddr *addr;

    /**
     * @brief bind flag
     */
    int binded;

    /**
     * @brief broadcast socket
     */
    socket_base_t *cast;

    /**
     * @brief socket property
     */
    property_t *property;
};
#define cast_ip   this->ip
#define cast_cast this->cast 
#define cast_port this->port
#define cast_addr this->addr
#define cast_bind this->binded
#define cast_property  this->property

METHOD(cast_t, send_, int, private_cast_t *this, void *buf, int size)
{
    return cast_cast->sendto(cast_cast, buf, size, 0, cast_addr, sizeof(struct sockaddr));
}

METHOD(cast_t, recv_, int, private_cast_t *this, void *buf, int size)
{
    int len = sizeof(struct sockaddr);
    if (!cast_bind) {
        if (bind(cast_cast->get_fd(cast_cast), cast_addr, sizeof(struct sockaddr)) < 0) return -1;
        cast_bind = 1;
    }
    return cast_cast->recvfrom(cast_cast, buf, size, 0, cast_addr, &len);
}

METHOD(cast_t, recv__, int, private_cast_t *this, void *buf, int size)
{
    int len = sizeof(struct sockaddr);

    if (!cast_bind) {
        struct ip_mreq mreq;

        mreq.imr_multiaddr.s_addr = inet_addr(cast_ip);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (cast_property->add_to_membership(cast_property, &mreq) < 0) return -1;

        cast_property->make_multicast_loop(cast_property, 1);
        cast_property->make_multicast_ttl(cast_property, 5);
        if (bind(cast_cast->get_fd(cast_cast), cast_addr, sizeof(struct sockaddr)) < 0) return -1;
        cast_bind = 1;
    }
    return cast_cast->recvfrom(cast_cast, buf, size, 0, cast_addr, &len);
}

METHOD(cast_t, destroy_, void, private_cast_t *this)
{
    if (cast_cast) cast_cast->destroy(cast_cast);
    if (cast_property) cast_property->destroy(cast_property);
    if (cast_ip) free(cast_ip);

    free(this);
}

static int broadcast_init(private_cast_t *this)
{
    int status = -1;

    status = cast_cast->socket(cast_cast, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (status <= 0) {
        perror("socket()");
        return -1;
    }

    cast_addr = cast_cast->init_addr(cast_cast, AF_INET, BROADCAST_IP, cast_port);
    cast_property->set_fd(cast_property, cast_cast->get_fd(cast_cast));
    status = cast_property->make_broadcast(cast_property, 1);
    if (status < 0) {
        perror("make_broadcast failed\n");
        return -1;
    }

    return 0;
}

cast_t *create_broadcast(const char *ip, int port)
{
    private_cast_t *this;

    INIT(this, 
        .public = {
            .send = _send_,
            .recv = _recv_,
            .destroy = _destroy_,
        },
        .ip   = strdup(ip),
        .port = port,
        .cast = create_socket_base(),
        .binded = 0,
        .property = create_property(-1),
    );

    if (broadcast_init(this) < 0) {
        _destroy_(this);
        return NULL;
    }

    return &this->public;
}

static int multicast_init(private_cast_t *this)
{
    int status = -1;

    status = cast_cast->socket(cast_cast, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (status <= 0) {
        perror("socket()");
        return -1;
    }

    cast_addr = cast_cast->init_addr(cast_cast, AF_INET, cast_ip, cast_port);
    cast_property->set_fd(cast_property, cast_cast->get_fd(cast_cast));
    if (status < 0) {
        perror("make_broadcast failed\n");
        return -1;
    }

    return 0;
}

cast_t *create_multicast(const char *ip, int port)
{
    private_cast_t *this;

    INIT(this, 
        .public = {
            .send = _send_,
            .recv = _recv__,
            .destroy = _destroy_,
        },
        .ip = strdup(ip),
        .port = port,
        .cast = create_socket_base(),
        .binded = 0,
        .property = create_property(-1),
    );

    if (multicast_init(this) < 0) {
        _destroy_(this);
        return NULL;
    }

    return &this->public;
}
