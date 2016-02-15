#include <listener.h>
#include <socket.h>
#include <socket_base.h>
#include <event.h>
#include <host.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <utils.h>
#include <thread.h>
#include <mutex.h>
#include <bsem.h>

typedef struct private_listener_t private_listener_t;
struct private_listener_t {
    /**
     * @brief public interface
     */
    listener_t public;

    /**
     * @brief net
     */
    socket_base_t *net;

    /**
     * @brief server_type
     */
    server_type_t type;

    /**
     * @brief socket event
     */
    event_t *evt;
};
#define network  this->net 
#define net_evt  this->evt
#define net_type this->type 
#define net_on_accept          this->public.on_accept
#define net_on_accept_handler  this->public.on_accept->handler
#define net_on_accept_arg      this->public.on_accept->arg
#define net_on_connect         this->public.on_connect
#define net_on_connect_handler this->public.on_connect->handler
#define net_on_connect_arg     this->public.on_connect->arg
#define net_on_recv            this->public.on_recv
#define net_on_recv_handler    this->public.on_recv->handler
#define net_on_recv_arg        this->public.on_recv->arg
#define net_on_close           this->public.on_close
#define net_on_close_handler   this->public.on_close->handler
#define net_on_close_arg       this->public.on_close->arg

static void on_recv_handler(int fd, private_listener_t *this)
{
    if (net_on_recv && net_on_recv_handler) net_on_recv_handler(fd, net_on_recv_arg);
}

static void on_close_handler(int fd, private_listener_t *this)
{
    net_evt->delete(net_evt, fd, EVENT_ON_ALL);
    if (net_on_close && net_on_close_handler) net_on_close_handler(fd, net_on_close_arg);
}

static void on_accept_handler(int fd, private_listener_t *this)
{
    int accept_fd = 0;
    accept_fd = network->accept(network, NULL);
    if (net_on_accept && net_on_accept_handler) net_on_accept_handler(fd, net_on_accept_arg);
    net_evt->add(net_evt, accept_fd, EVENT_ON_RECV, (void *)on_recv_handler, this);
    net_evt->add(net_evt, accept_fd, EVENT_ON_CLOSE, (void *)on_close_handler, this);
}

METHOD(listener_t, listen_, int, private_listener_t *this, int family, int type, char *ip, int port)
{
    int fd = 0;
    
    if (!ip || port < 1) return -1;
    
    /**
     * start server
     */
    network = create_socket_base();
    if (!network) return -1;
    fd = network->socket(network, family, type, 0);
    if (fd < 1) return -1;
    if (network->bind(network, ip, port) < 0) return -1;
    if (network->listen(network, 5) < 0) return -1;

    /**
     * start socket event 
     */
    net_evt = event_create(EVENT_MODE_EPOLL, 1000);
    if (!net_evt) return -1;
    net_evt->add(net_evt, fd, EVENT_ON_ACCEPT, (void *)on_accept_handler, this);

    return 0;
}

METHOD(listener_t, connect_, int, private_listener_t *this, int family, int type, char *ip, int port)
{
    int fd = 0;
    
    if (!ip || port < 1) return -1;
    
    /**
     * start server
     */
    network = create_socket_base();
    if (!network) return -1;
    fd = network->socket(network, family, type, 0);
    if (fd < 1) return -1;
    if (network->connect(network, ip, port) < 0) return -1;
    if (net_on_connect && net_on_connect_handler) net_on_connect_handler(fd, net_on_connect_arg);

    /**
     * start socket event 
     */
    net_evt = event_create(EVENT_MODE_EPOLL, 1000);
    if (!net_evt) return -1;
    net_evt->add(net_evt, fd, EVENT_ON_RECV, (void *)on_recv_handler, this);
    net_evt->add(net_evt, fd, EVENT_ON_CLOSE, (void *)on_close_handler, this);

    return 0;
}

METHOD(listener_t, close_, int, private_listener_t *this)
{
    return network->close(network);
}

METHOD(listener_t, destroy_, void, private_listener_t *this)
{
    if (network) network->destroy(network);
    if (net_evt) net_evt->destroy(net_evt);
    if (net_on_accept)  free(net_on_accept);
    if (net_on_connect) free(net_on_connect);
    if (net_on_recv)    free(net_on_recv);
    if (net_on_close)   free(net_on_close);
    free(this);
}

METHOD(listener_t, set_cb, void, private_listener_t *this, on_type_t type, listener_handler handler, void *arg)
{
    switch (type) {
        case ON_ACCEPT:
            if (net_on_accept) free(net_on_accept);
            net_on_accept  = NULL;
            net_on_accept  = create_listener_cb(handler, arg);
            break;
        case ON_CONNECT:
            if (net_on_connect) free(net_on_connect);
            net_on_connect = NULL;
            net_on_connect = create_listener_cb(handler, arg);
            break;
        case ON_RECV:
            if (net_on_recv) free(net_on_recv);
            net_on_recv    = NULL;
            net_on_recv    = create_listener_cb(handler, arg);
            break;
        case ON_CLOSE:
            if (net_on_close) free(net_on_close);
            net_on_close   = NULL;
            net_on_close   = create_listener_cb(handler, arg);
            break;
        default:
            break;
    }
}

listener_t *create_listener()
{
    private_listener_t *this;

    INIT(this, 
        .public = {
            .listen  = _listen_,
            .connect = _connect_,
            .close   = _close_,
            .destroy = _destroy_,
            .set_cb  = _set_cb,
            
            .on_accept  = NULL,
            .on_connect = NULL,
            .on_recv    = NULL,
            .on_close   = NULL,
        },
        .net  = NULL,
        .evt  = NULL,
        .type = -1,
    );

    return &this->public;
}

listener_cb_t *create_listener_cb(void (*handler) (int , void *), void *arg)
{
    listener_cb_t *this;

    INIT(this, 
        .handler = handler,
        .arg     = arg,
    );

    return this;
}
