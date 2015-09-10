#include "sock.h"
#define UNIX_DOMAIN "/tmp/unix.sock"

void on_recv(int cli_fd, void *arg)
{
    //char buf[128] = {0};
    //sock_data_t data = {0};
    //int num = 222;
    int value = 0;
    socket_data_type type = -1;

    socket_data_recv(cli_fd, &type, &value, sizeof(value));
    printf("recv: %d\n", value);
    
    if (socket_addr_data_sendto(cli_fd, INT, &value, sizeof(value), &((struct socket *)arg)->addr) < 0)
        perror("sendto");

    //socket_data_send(cli_fd, INT, &num, sizeof(num));
    /*
    if ((value = socket_data_sendto(cli_fd, INT, &num, sizeof(num), "172.21.22.89", 5001)) > 0)
        printf("send count: %d\n", value);
    else perror("send error");
    */

    return;
}

void on_close(int cli_fd, void *arg)
{
    printf("on_close\n");
    return;
}

void on_accept(int cli_fd, void *arg)
{
    printf("on_accpet\n");
}

int start_data_capture();
#define macro_to_str(x) #x


int main(int agrc, char *agrv[])
{
    struct socket ser = {0};
    u_short port = 5001;
    //char *ip = NULL;

    //start_data_capture();
    //return 0;

    char buf[128] = {0};
    if (get_ifname(buf) != -1)
        printf("%s\n", buf);;
    //udp_broadcast_recv("255.255.255.255", 5001, 1, &pp, 128);
    //udp_broadcast_recv("172.21.22.255", 5001, 1, &pp, 128);
    udp_multicast_recv("224.0.0.1", 5001, 2, buf, 128);
    printf("%s\n", buf);

    return 0;
    //get_ip(ip);
    server_create(&ser, AF_INET, SOCK_STREAM, port, NULL);
    socket_event_add(&ser.evl, SOCKET_ON_ACCEPT, on_accept, NULL);
    socket_event_add(&ser.evl, SOCKET_ON_RECV, on_recv, &ser);
    socket_event_add(&ser.evl, SOCKET_ON_CLOSE, on_close, NULL);

    /*
    socket_data_type type = -1;
    int value = 0;
    udp_socket_data_recv(ser.fd, &type, &value, sizeof(value));
    printf("type = %d, value = %d\n", type, value);
    */

    while (1);

    return 0;
}

/**
 * @brief printf socket error
 *
 * @param sc [in] string
 */
static void error_die(const char *sc)
{
    perror(sc);
    exit(1);
}

/*************************************************************
**  Function Declaration Of TCP Socket Background Service  ***
**************************************************************/
static  void* tcp_server_backup_service(void *sock);
static  void* tcp_client_backup_service(void *sock);

/*************************************************************
**  Function Declaration Of UDP Socket Background Service  ***
**************************************************************/
static  void* udp_backup_service(void *sock);


/*************************************************************
*********  Function Declaration Of Socket Basic  *************
**************************************************************/
/**
 * @brief create a soocket
 *
 * @param domain [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 *
 * @return socket fd, if succ; -1, if failed.
 */
int socket_create(int domain, int type)
{
    int fd = socket(domain, type, 0);
    if (fd == -1) error_die("socket");
    return fd;
}

/**
 * @brief init sockaddr_in
 *
 * @param addr   [in]
 * @param domain [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param port   [in] port
 * @param ip     [in] ip address
 */
void socket_addr_init(void *addr, int domain, u_short port, const char *ip)
{
    /**
     * #define   AF_UNIX    1       local   to   host   (pipes,   portals)
     * #define   AF_INET    2       internetwork:   UDP,   TCP,   etc.
     */
    if (domain == AF_UNIX || domain == AF_LOCAL)
    {
        /* exit if ip is NULL */
        if (ip == NULL) 
        {
            printf("AF_UNIX path can't be NULL.\n");
            exit(1);
        }

        /**
         * init struct of socket address
         */
        struct sockaddr_un *addr_un = (struct sockaddr_un *)addr;
        addr_un->sun_family = domain;
        strcpy(addr_un->sun_path, ip);
    }
    else
    {
        /**
         * init struct of socket address
         */
        struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
        addr_in->sin_family = domain;
        if (ip == NULL) addr_in->sin_addr.s_addr = htonl(INADDR_ANY);
        else addr_in->sin_addr.s_addr = inet_addr(ip);
        addr_in->sin_port = htons(port);
    }

    return;
}

/**
 * @brief bind a socket。
 *
 * @param addr [in]
 * @param fd   [in] fd of server socket。
 *
 * @return 0, if succ; -1, if failed.
 */
int socket_bind(int fd, void *addr)
{
    int addr_len = sizeof(struct sockaddr);

    /* socket bind */
    if (bind(fd,(struct sockaddr *)addr, addr_len) < 0)
        error_die("bind");

    return 0;
}

/**
 * @brief  listening for an incoming connect.
 *
 * @param fd      [in] listen socket fd.
 * @param backlog [in] Maximum length of the queue of pending connections.
 *
 * @return 0, if succ; -1, if failed.
 */
int socket_listen(int fd, int backlog)
{
    if (listen(fd, backlog) < 0) error_die("listen");

    return 0;
}

/**
 * @brief start up a server socket
 *
 * @param domain [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param port   [in] socket port
 * @param ip     [in] ip address
 *
 * @return socket fd, if succ; exit, if fail
 */
int socket_startup(int domain, int type, void *addr, u_short port, \
        const char *ip, int is_ser)
{
    int fd = -1;

    /* create socket */
    fd = socket_create(domain, type);
    make_listen_socket_reuseable(fd);
    
    /* init sockaddr_in */
    socket_addr_init(addr, domain, port, ip);
    
    /* bind socket */
    if (is_ser) socket_bind(fd, addr);
    
    /* build socket listen */
    if (is_ser && type == SOCK_STREAM) socket_listen(fd, 5);

    return fd;
}

/**
 * @brief Waiting for an incoming socket.
 *
 * @param fd        [in] server socket fd.
 *
 * @return new socket fd, if succ; -1, if failed.
 */
int socket_accept(int fd)
{
    struct sockaddr cli_addr;
    socklen_t len = sizeof(struct sockaddr);
    
    return accept(fd, &cli_addr, &len); 
}

/**
 * @brief Connect to server.
 *
 * @param fd       [in] client fd
 * @param cli_addr [in] client sockaddr_in
 *
 * @return 
 */
int socket_connect(int fd, void *cli_addr)
{
    return connect(fd, (struct sockaddr *)cli_addr, sizeof(struct sockaddr));
}

/**
 * @brief Connect to server with a timeout.
 *
 * @param fd       [in] client fd
 * @param cli_addr [in] client sockaddr_in
 * @param tm_ms    [in] time out
 *
 * @return 0, if succ; -1, if fail
 */
int socket_time_connect(int fd, void *cli_addr, int tm_ms)
{
    struct timeval tv = {0};
    fd_set wfd;
    int rt = -1;
    int len;
    int interval = 100;

    if (fd < 0) return -1;

    /* make socket non blocking */
    make_socket_nonblock(fd);
    
    /* connect to server looply */
    while (1) 
    {
        /* empty fd sets */
        FD_ZERO(&wfd);
        FD_SET(fd, &wfd);

        /* timer */
        tv.tv_sec = 0;
        tv.tv_usec = 1000 * interval;

        /* connect to server */
        rt = connect(fd, (struct sockaddr *)cli_addr, sizeof(struct sockaddr));
        if (rt == 0 || errno != EINPROGRESS) break;

        /* detect whether is connected successfully */
        if (select(fd + 1, NULL, &wfd, NULL, &tv) > 0) 
        {
            if (FD_ISSET(fd, &wfd))
            {
                len = sizeof(int);
                if (!getsockopt(fd, SOL_SOCKET, SO_ERROR, NULL, NULL))
                {
                    rt = 0;
                    break;
                }
            }
        }

        /* timer decline */
        tm_ms -= interval;
        if (tm_ms <= 0) 
        {
            rt = -1;
            break;
        }
    }

    /* close socket if connected failed */
    if (rt < 0) close(fd);

    return rt;
}

/**
 * @brief close a socket
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if failed.
 */
int socket_close(int fd)
{
    return close(fd);
}


/*************************************************************
*********  Function Declaration Of TCP Socket Send  **********
**************************************************************/

/**
 * @brief send a message
 *
 * @param fd  [in] socket
 * @param buf  [in] message buffer
 * @param size [in] size of message buffer
 *
 * @return size of message sended, if succ; -1, if failed.
 */
int socket_send(int fd, void *buf, int size)
{
    return send(fd, buf, size, 0);
}

/**
 * @brief send a message
 *
 * @param fd        [in] socket
 * @param buf       [in] message buffer
 * @param size      [in] size of message buffer
 * @param time_ms   [in] timeout
 *
 * @return size of message sended, if succ; -1, if failed.
 */
int socket_time_send(int fd, void *buf, int size, int time_ms)
{
    int rt = -1;
    
    make_socket_send_timeout(fd, time_ms);
    rt = send(fd, buf, size, 0);
    make_socket_send_timeout(fd, 0);

    return rt;
}

/**
 * @brief send socket data
 *
 * @param fd   [in] socket fd
 * @param type [in] socket data type
 * @param buf  [in] send buffer
 * @param size [in] size of send buffer
 *
 * @return size of data sending, if succ; -1, if fail
 */
int socket_data_send(int fd, socket_data_type type, void *buf, int size)
{
    socket_data_t sck_data = {0};
    int send_size = 0;

    send_size = (size < sizeof(sck_data.value)) ? size : sizeof(sck_data.value);
    if (buf != NULL) memcpy(&sck_data.value, buf, send_size);
    sck_data.type = type;
    sck_data.size = send_size;
    
    return send(fd, &sck_data, SOCKET_DATA_HEADER_SIZE + send_size, 0);
}


/*************************************************************
*********  Function Declaration Of UDP Socket Send  **********
**************************************************************/
/**
 * @brief send data with udp socket
 *
 * @param fd    [in] socket fd
 * @param buf   [in] send buffer
 * @param size  [in] size of send buffer
 * @param ip    [in] ip which want to send
 * @param port  [in] port which want to send
 *
 * @return size of data sended, if succ; -1, if failed.
 */
int socket_sendto(int fd, void *buf, int size, const char *ip, int port)
{
    struct sockaddr_in addr = {0};
    int len = 0;
    
    if (fd < 0) return -1;

    addr.sin_family = AF_INET;
    if (ip != NULL)
        addr.sin_addr.s_addr = inet_addr(ip);
    else 
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    len = sizeof(addr);
    return sendto(fd, buf, size, 0, (struct sockaddr *)&addr, (socklen_t)len);
}

/**
 * @brief send data with udp socket
 *
 * @param fd    [in] socket fd
 * @param buf   [in] send buffer
 * @param size  [in] size of send buffer
 * @param addr  [in] addr struct of udp socket
 *
 * @return size of data sended, if succ; -1, if failed.
 */
int socket_addr_sendto(int fd, void *buf, int size, void *addr)
{
    int len = sizeof(struct sockaddr);

    return sendto(fd, buf, size, 0, (struct sockaddr *)addr, (socklen_t)len);
}

/**
 * @brief send data with udp socket
 *
 * @param fd    [in] socket fd
 * @param type [in] socket data type
 * @param buf   [in] send buffer
 * @param size  [in] size of send buffer
 * @param ip    [in] ip which want to send
 * @param port  [in] port which want to send
 *
 * @return size of data sended, if succ; -1, if failed.
 */
int socket_data_sendto(int fd, socket_data_type type, \
            void *buf, int size, const char *ip, int port)
{
    struct sockaddr_in addr = {0};
    socket_data_t sck_data = {0};
    int send_size = 0;
    int len = 0;   

    /* init socket data */
    send_size = (size < sizeof(sck_data.value)) ? size : sizeof(sck_data.value);
    if (buf != NULL) memcpy(&sck_data.value, buf, send_size);
    sck_data.type = type;
    sck_data.size = send_size;

    /* init address info of udp socket */
    addr.sin_family = AF_INET;
    if (ip != NULL)
        addr.sin_addr.s_addr = inet_addr(ip);
    else 
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    /* send */
    len = sizeof(addr);
    return sendto(fd, &sck_data, SOCKET_DATA_HEADER_SIZE + send_size, \
            0, (struct sockaddr *)&addr, (socklen_t)len);
}

/**
 * @brief send data with udp socket
 *
 * @param fd    [in] socket fd
 * @param type [in] socket data type
 * @param buf   [in] send buffer
 * @param size  [in] size of send buffer
 * @param addr  [in] addr struct of udp socket
 *
 * @return size of data sended, if succ; -1, if failed.
 */
int socket_addr_data_sendto(int fd, socket_data_type type, \
            void *buf, int size, void *addr)
{
    socket_data_t sck_data = {0};
    int send_size = 0;
    int len = sizeof(struct sockaddr);

    /* init address info of udp socket */
    send_size = (size < sizeof(sck_data.value)) ? size : sizeof(sck_data.value);
    if (buf != NULL) memcpy(&sck_data.value, buf, send_size);
    sck_data.type = type;
    sck_data.size = send_size;

    return sendto(fd, &sck_data, SOCKET_DATA_HEADER_SIZE + send_size, \
            0, (struct sockaddr *)addr, (socklen_t)len);
}


/*************************************************************
*********  Function Declaration Of TCP Socket Recv  **********
**************************************************************/
/**
 * @brief recveive a message
 *
 * @param fd   [in] socket
 * @param buf  [in] message buffer
 * @param size [in] size of message buffer
 *
 * @return size of message recveived, if succ; -1, if failed.
 */
int socket_recv(int fd, void *buf, int size)
{
    return recv(fd, buf, size, 0);
}

/**
 * @brief recveive a message
 *
 * @param fd        [in] socket
 * @param buf       [in] message buffer
 * @param size      [in] size of message buffer
 * @param time_ms   [in] timeout
 *
 * @return size of message recveived, if succ; -1, if failed.
 */
int socket_time_recv(int fd, void *buf, int size, int time_ms)
{
    struct timeval tv = {0};
    fd_set fds;
    int can_recv_bytes = 0;
    int rt = -1;

    /* empty fd sets */
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    
    /* timer */
    tv.tv_sec = time_ms / 1000;
    tv.tv_usec = time_ms % 1000;
    select(fd + 1, &fds, NULL, NULL, &tv);

    /* recv */
    if (FD_ISSET(fd, &fds)) 
    {
        can_recv_bytes = get_can_read_bytes(fd);
        if (can_recv_bytes > size)
            rt = recv(fd, buf, size, 0);
        else rt = recv(fd, buf, can_recv_bytes, 0);
    }

    return rt;
}

/**
 * @brief recv socket data
 *
 * @param fd   [in] socket fd
 * @param type [in] socket data type
 * @param buf  [in] recv buffer
 * @param size [in] size of recv buffer
 *
 * @return size of data recving, if succ; -1, if fail
 */
int socket_data_recv(int fd, socket_data_type *type, void *buf, int size)
{
    socket_data_t sck_data = {0};
    int head_size = SOCKET_DATA_HEADER_SIZE;
    int can_read_bytes = 0;
    int recv_size = 0;
    
    /* receive when the read buffer is greater than 0 */
    while ((can_read_bytes = get_can_read_bytes(fd)) == 0) usleep(10);
    
    /* socket data recv */
    if (can_read_bytes >= head_size) 
    {
        /* recv header of struct data */
        recv(fd, &sck_data, head_size, 0);
        if (type != NULL) *type = sck_data.type;
        recv_size = ( (can_read_bytes - head_size) < size ) \
                    ? ( can_read_bytes - head_size ) : size;
    }
    else recv_size = size;

    /* recv socket data */
    return recv(fd, buf, recv_size, 0);
}

/**
 * @brief recv socket data
 *
 * @param fd      [in] socket fd
 * @param type    [in] socket data type
 * @param buf     [in] recv buffer
 * @param size    [in] size of recv buffer
 * @param time_ms [in] timeout of recv
 *
 * @return size of data recving, if succ; -1, if fail
 */
int socket_data_time_recv(int fd, socket_data_type *type, \
        void *buf, int size, int time_ms)
{
    socket_data_t sck_data = {0};
    struct timeval tv = {0};
    fd_set fds;
    int head_size = SOCKET_DATA_HEADER_SIZE;
    int can_read_bytes = 0;
    int recv_size = 0;

    /* empty fd sets */
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    
    /* timer */
    tv.tv_sec = time_ms / 1000;
    tv.tv_usec = time_ms % 1000;
    select(fd + 1, &fds, NULL, NULL, &tv);

    if (FD_ISSET(fd, &fds))
    {
        can_read_bytes = get_can_read_bytes(fd);
        if (can_read_bytes >= head_size) 
        {
            /* recv header of struct data */
            recv(fd, &sck_data, head_size, 0);
            if (type != NULL) *type = sck_data.type;
            recv_size = ( (can_read_bytes - head_size) < size ) ?\
                        ( can_read_bytes - head_size ) : size;
        }
        else recv_size = size;

        /* recv socket data */
        return recv(fd, buf, recv_size, 0);
    }

    return -1;
}


/*************************************************************
*********  Function Declaration Of UDP Socket Recv  **********
**************************************************************/
/**
 * @brief recv socket data
 *
 * @param fd   [in] socket fd with udp socket
 * @param type [in] socket data type
 * @param buf  [in] recv buffer
 * @param size [in] size of recv buffer
 *
 * @return size of data recving, if succ; -1, if fail
  */
int udp_socket_data_recv(int fd, socket_data_type *type, void *buf, int size)
{
    socket_data_t sck_data = {0};
    int head_size = SOCKET_DATA_HEADER_SIZE;
    int can_read_bytes = 0;
    int recv_size = 0;
    
    /* receive when the read buffer is greater than 0 */
    while ((can_read_bytes = get_can_read_bytes(fd)) == 0) usleep(10);
    
    /* socket data recv */
    if (can_read_bytes >= head_size) 
    {
        /* recv data */
        recv_size = recv(fd, &sck_data, can_read_bytes, 0);
        if (type != NULL) *type = sck_data.type;
        memcpy(buf, &sck_data.value, can_read_bytes);

        return recv_size;
    }
    else recv_size = size;

    return recv(fd, buf, recv_size, 0);
}

/**
 * @brief recveive a message with a udp socket
 *
 * @param fd    [in] socket fd
 * @param buf   [in] send buffer
 * @param size  [in] size of send buffer
 * @param ip    [in] ip which want to send
 * @param port  [in] port which want to send
 *
 * @return size of data sended, if succ; -1, if failed.
 */
int socket_recvfrom(int fd, void *buf, int size, const char *ip, int port)
{
    struct sockaddr_in addr = {0};
    int len = sizeof(struct sockaddr_in);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    return recvfrom(fd, buf, size, 0, \
            (struct sockaddr *)&addr, (socklen_t *)&len);
}

/**
 * @brief recv data with udp socket
 *
 * @param fd    [in] socket fd
 * @param buf   [in] send buffer
 * @param size  [in] size of send buffer
 * @param addr  [in] addr struct of udp socket
 *
 * @return size of data sended, if succ; -1, if failed.
 */
int socket_addr_recvfrom(int fd, void *buf, int size, void *addr)
{
    int len = sizeof(struct sockaddr);

    return recvfrom(fd, buf, size, 0, \
            (struct sockaddr *)addr, (socklen_t *)&len);
}


/**
 * @brief recv socket data
 *
 * @param fd   [in] socket fd
 * @param type [in] socket data type
 * @param buf  [in] recv buffer
 * @param size [in] size of recv buffer
 * @param ip   [in] ip which want to send
 * @param port [in] port which want to send
 *
 * @return size of data recving, if succ; -1, if fail
 */
int socket_data_recvfrom(int fd, socket_data_type *type, \
            void *buf, int size, const char *ip, int port)
{
    struct sockaddr_in addr = {0};
    socket_data_t sck_data = {0};
    int head_size = SOCKET_DATA_HEADER_SIZE;
    int len = sizeof(struct sockaddr_in);
    int can_read_bytes = 0;
    int recv_size = 0;

    /* init address of socket */
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    
    /* receive when the read buffer is greater than 0 */
    while ((can_read_bytes = get_can_read_bytes(fd)) == 0) usleep(10);
    
    /* socket data recv */
    if (can_read_bytes >= head_size) 
    {
        /* recv data */
        recv_size = recvfrom(fd, &sck_data, can_read_bytes, \
                    0, (struct sockaddr *)&addr, (socklen_t *)&len);
        if (type != NULL) *type = sck_data.type;
        recv_size = ((recv_size - head_size) < size) ? \
                    (recv_size - head_size) : size;
        memcpy(buf, &sck_data.value, recv_size);

        return recv_size;
    }
    else recv_size = size;

    return recvfrom(fd, buf, recv_size, 0, \
                (struct sockaddr *)&addr, (socklen_t *)&len);
}

/**
 * @brief recv socket data
 *
 * @param fd   [in] socket fd
 * @param type [in] socket data type
 * @param buf  [in] recv buffer
 * @param size [in] size of recv buffer
 * @param addr [in] addr struct
 *
 * @return size of data recving, if succ; -1, if fail
 */
int socket_addr_data_recvfrom(int fd, socket_data_type *type, \
                void *buf, int size, void *addr)
{
    socket_data_t sck_data = {0};
    int head_size = SOCKET_DATA_HEADER_SIZE;
    int len = sizeof(struct sockaddr);
    int can_read_bytes = 0;
    int recv_size = 0;

    /* receive when the read buffer is greater than 0 */
    while ((can_read_bytes = get_can_read_bytes(fd)) == 0) usleep(10);
    
    /* socket data recv */
    if (can_read_bytes >= head_size) 
    {
        /* recv data */
        recv_size = recvfrom(fd, &sck_data, can_read_bytes, \
                    0, (struct sockaddr *)addr, (socklen_t *)&len);
        if (type != NULL) *type = sck_data.type;
        recv_size = ((recv_size - head_size) < size) ? (recv_size - head_size) : size;
        memcpy(buf, &sck_data.value, recv_size);

        return recv_size;
    }
    else recv_size = size;

    return recvfrom(fd, buf, recv_size, 0, \
            (struct sockaddr *)addr, (socklen_t *)&len);
}


/*************************************************************
*********  Function Declaration Of Socket Event  *************
**************************************************************/

/**
 * @brief socket event init 
 *
 * @param evl [in] event loop
 */
void socket_event_init(event_loop_t *evl)
{
    if (evl == NULL) return;

    memset(evl, 0, sizeof(event_loop_t));

    return;
}

/**
 * @brief add socket event
 *
 * @param evl  [in] event loop
 * @param evt  [in] event type
 * @param cb   [in] event callback
 * @param arg  [in] parameter of callback
 */
void socket_event_add(event_loop_t *evl, event_type_t evt, event_cb cb, void *arg)
{
    if (evl == NULL) return;

    switch(evt)
    {
        case SOCKET_ON_ACCEPT :
            evl->on_accept.evt_cb = cb;
            evl->on_accept.arg = arg;
            break;
        case SOCKET_ON_CONNECT :
            evl->on_connect.evt_cb = cb;
            evl->on_connect.arg = arg;
            break;
        case SOCKET_ON_SEND :
            evl->on_send.evt_cb = cb;
            evl->on_send.arg = arg;
            break;
        case SOCKET_ON_RECV :
            evl->on_recv.evt_cb = cb;
            evl->on_recv.arg = arg;
            break;
        case SOCKET_ON_CLOSE :
            evl->on_close.evt_cb = cb;
            evl->on_close.arg = arg;
            break;
        default:
            break;
    }
}

/**
 * @brief delete a socket event
 *
 * @param evl [in] event loop
 * @param evt [in] event type
 */
void socket_event_delete(event_loop_t *evl, event_type_t evt)
{
    if (evl == NULL) return;

    switch(evt)
    {
        case SOCKET_ON_ACCEPT :
            evl->on_accept.evt_cb = NULL;
            evl->on_accept.arg = NULL;
            break;
        case SOCKET_ON_CONNECT :
            evl->on_connect.evt_cb = NULL;
            evl->on_connect.arg = NULL;
            break;
        case SOCKET_ON_SEND :
            evl->on_send.evt_cb = NULL;
            evl->on_send.arg = NULL;
            break;
        case SOCKET_ON_RECV :
            evl->on_recv.evt_cb = NULL;
            evl->on_recv.arg = NULL;
            break;
        case SOCKET_ON_CLOSE :
            evl->on_close.evt_cb = NULL;
            evl->on_close.arg = NULL;
            break;
        default:
            break;
    }

    return;
}

/**
 * @brief clear all socket events
 *
 * @param evl [in] event loop
 */
void socket_event_clearall(event_loop_t *evl)
{
    if (evl == NULL) return;

    evl->on_accept.evt_cb = NULL;
    evl->on_accept.arg = NULL;

    evl->on_connect.evt_cb = NULL;
    evl->on_connect.arg = NULL;

    evl->on_send.evt_cb = NULL;
    evl->on_send.arg = NULL;

    evl->on_recv.evt_cb = NULL;
    evl->on_recv.arg = NULL;

    evl->on_close.evt_cb = NULL;
    evl->on_close.arg = NULL;

    return;
}

/**
 * @brief process socket event
 *
 * @param fd [in] socket fd
 * @param cb [in] event callback
 */
void socket_event_process(int fd, callback cb)
{
    if (cb.evt_cb == NULL) return;

    cb.evt_cb(fd, cb.arg);

    return;
}


/*************************************************************
*****  Function Declaration Of Socket Property Settings  *****
**************************************************************/
/**
 * @brief get local machine's ip address.
 * 
 * @param ip[] [out] local ip address.
 *
 * @return 0, if succ; -1, if failed.
 */
int get_ip(char ip[])
{
    struct ifaddrs *ifaddr = NULL;
    void *tmp_addr = NULL;
    char tmp_ip[20] = {0};
    char *hname = NULL;

    if (ip == NULL) return -1;

    strcpy(ip, "\0");
    getifaddrs(&ifaddr);
    while (ifaddr != NULL) 
    {
        if (ifaddr->ifa_addr->sa_family == AF_INET) 
        {
            tmp_addr = &((struct sockaddr_in *)ifaddr->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmp_addr, tmp_ip, INET_ADDRSTRLEN);
            hname = ifaddr->ifa_name;
            if (hname[0] != 'l') strncpy(ip, tmp_ip, 20);
        }

        ifaddr = ifaddr->ifa_next;
    }

    freeifaddrs(ifaddr);

    return 0;
}

/**
 * @brief get ip address by ifname 
 *
 * @param ifname [in]  interface name
 * @param ip     [out] ip address 
 *
 * @return 0, if succ; -1, if failed
 */
int get_ip_by_ifname(const char *ifname, char *ip)
{
    int fd = -1;
    struct ifreq ifr;
    struct sockaddr_in addr = {0};
    if (ifname == NULL) return -1;

    strcpy(ifr.ifr_name, ifname);
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    strcpy(ip, "\0");
    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) 
    {
        memcpy(&addr, &ifr.ifr_addr, sizeof(ifr.ifr_addr));
        strcpy(ip, inet_ntoa(addr.sin_addr));
        close(fd);
        return 0;
    }

    close(fd);

    return -1;
}

/**
 a* @brief get subnet ip address 
 *
 * @param ip   [in] ip
 * @param mask [in] mask
 *
 * @return subnet ip address, if succ;
 */
char * get_subnet_addr(const char *ip, const char *mask)
{
    struct in_addr lip, lmask, subnet;

    if (ip == NULL || mask == NULL) return NULL;

    inet_aton(ip, &lip);
    inet_aton(mask, &lmask);

    subnet.s_addr = lip.s_addr & lmask.s_addr;

    return strdup(inet_ntoa(subnet));
}

/**
 * @brief convert mask to bits of mask
 *
 * @param mask [in] mask address
 *
 * @return bits of mask, if succ; -1, if failed
 */
int mask_to_bits(const char *mask)
{
    int i = 0;
    int n = 0;
    struct in_addr addr;
    int bits = sizeof(unsigned int) * 8;

    if (!match_ip(mask)) return -1;

    inet_pton(AF_INET, mask, &addr);
    for (i = bits - 1; i >=0; i--)
    {
        if (addr.s_addr & (0x01 << i))
            n++;
    }

    return n;
}

/**
 * @brief get bytes which can be readed in the recvive buffer.
 *
 * @param fd [in] socket fd
 *
 * @return data bytes which can be readed, if succ; -1, if failed.
 */
int get_can_read_bytes(int fd)
{
    int can_read_bytes = -1;

    ioctl(fd, FIONREAD, &can_read_bytes);

    return can_read_bytes;
}

/**
 * @brief set socket unblock 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1 , if fail
 */
int make_socket_nonblock(int fd)
{
    int flag = 0;
    if ((flag = fcntl(fd, F_GETFL, 0)) < 0)
    {
        perror("fcntl F_GETFL");
        return -1;
    }

    if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) < 0)
    {
        perror("fcntl F_SETFL");
        return -1;
    }

    return 0;
}

/**
 * @brief set socket block 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1 , if fail
 */
int make_socket_block(int fd)
{
    int flag = 0;
    if ((flag = fcntl(fd, F_GETFL, 0)) < 0)
    {
        perror("fcntl F_GETFL");
        return -1;
    }

    if (fcntl(fd, F_SETFL, flag & ~O_NONBLOCK) < 0)
    {
        perror("fcntl F_SETFL");
        return -1;
    }

    return 0;
}

/**
 * @brief make listen socket reuseable 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if fail
 */
int make_listen_socket_reuseable(int fd)
{
    int on = 1;

    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
}

/**
 * @brief make socket keep alive 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if fail
 */
/*int make_socket_keep_alive(int fd)
{
    int on = 1;

    return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));
}*/

/**
 * @brief make socket child can't exec 
 *
 * @param fd [in] socket fd
 * 
 * @return 0, if succ; -1, if fail
 */
int make_socket_closenexec(int fd)
{
    int flags = 0;

    if ((flags = fcntl(fd, F_GETFD, NULL)) < 0) 
    {
        perror("fcntl F_GETFD");
        return -1;
    }

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
    {
        perror("fcntl F_SETFD");
        return -1;
    }

    return 0;
}

/**
 * @brief make socket keep alive 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if fail
 */
int make_socket_keep_alive(int fd)
{
    int keep_alive = 1;     /* open keep alive attribute */
    int keep_idle = 60;     /* if connection no data exchanges in \
				60 seconds, then detect */
    int keep_interval = 5;  /* when detecting, the time interval is 5 seconds */
    int keep_count = 3;     /* The number of attempts to detect. \
				If the first probe packet is received, then the 2 time no longer. */

    /**
     * open keepalive mechanism
     */
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, \
                &keep_alive, sizeof(keep_alive)) == -1)
    {
        perror("set keep alive");
        return -1;
    }

    /* Default settings are more or less garbage, with the keepalive time 
     * set to 7200 by default on Linux. Modify settings to make the feature 
     * actually useful. */  
  
    /* Send first probe after interval. */  
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, \
                &keep_idle, sizeof(keep_idle)) == -1)
    {
        perror("set keep idle");
        return -1;
    }

    /* Send next probes after the specified interval. Note that we set the 
     * delay as interval / 3, as we send three probes before detecting 
     * an error (see the next setsockopt call). */  
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, \
                &keep_interval, sizeof(keep_interval)) == -1)
    {
        perror("set keep interval");
        return -1;
    }

    /* Consider the socket in error state after three we send three ACK 
     * probes without getting a reply. */  
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, \
                &keep_count, sizeof(keep_count)) == -1)
    {
        perror("set keep idle");
        return -1;
    }

    return 0;
}

/**
 * @brief set size of socket recv buffer 
 *
 * @param fd        [in] socket fd
 * @param buf_size  [in] buffer size
 *
 * @return 0, if succ; -1, if failed.
 */
int set_socket_recv_buf(int fd, int buf_size)
{
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size)) < 0)
    {
        perror("set recv buf size");
        return -1;
    }

    return 0;
}

/**
 * @brief set size of socket send buffer 
 *
 * @param fd        [in] socket fd
 * @param buf_size  [in] buffer size
 *
 * @return 0, if succ; -1, if failed.
 */
int set_socket_send_buf(int fd, int buf_size)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size)) < 0)
    {
        perror("set send buf size");
        return -1;
    }

    return 0;
}

/**
 * @brief get size of socket recv buffer 
 *
 * @param fd        [in] socket fd
 *
 * @return recv buffer size, if succ; -1, if failed.
 */
int get_socket_recv_buf(int fd)
{
    int buf_size = 0;
    int len = sizeof(buf_size);

    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buf_size, \
                (socklen_t *)&len) < 0)
    {
        perror("get recv buf size");
        return -1;
    }

    return buf_size;
}

/**
 * @brief get size of socket send buffer 
 *
 * @param fd        [in] socket fd
 *
 * @return send buffer size, if succ; -1, if failed.
 */
int get_socket_send_buf(int fd)
{
    int buf_size = 0;
    int len = sizeof(buf_size);

    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buf_size, \
                (socklen_t *)&len) < 0)
    {
        perror("get send buf size");
        return -1;
    }

    return buf_size;
}

/**
 * @brief set socket close action 
 *
 * @param fd    [in] socket fd
 * @param is_on [in] swith of close action
 * @param tm_s  [in] time
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_close_action(int fd, int is_on, int tm_s)
{
    struct linger so_linger = {0};

    so_linger.l_onoff = is_on;
    so_linger.l_linger = tm_s;

    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, \
                &so_linger, sizeof(so_linger)))
    {
        perror("setsockopt so_linger");
        return -1;
    }

    return 0;
}

/**
 * @brief set socket broadcast 
 *
 * @param fd [in] socket fd
 * @param on [in] switch
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_broadcast(int fd, int on)
{
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, \
                &on, sizeof(on)))
    {
        perror("setsockopt so_broadcast");
        return -1;
    }

    return 0;

}

/**
 * @brief set socket multicast loop 
 *
 * @param fd [in] socket fd
 * @param on [in] switch
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_multicast_loop(int fd, int on)
{
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, \
                &on, sizeof(on)))
    {
        perror("setsockopt so_multicast_loop");
        return -1;
    }

    return 0;

}

/**
 * @brief set socket multicast ttl 
 *
 * @param fd  [in] socket fd
 * @param ttl [in] ttl
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_multicast_ttl(int fd, int ttl)
{
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, \
                &ttl, sizeof(ttl)))
    {
        perror("setsockopt so_multicast_ttl");
        return -1;
    }

    return 0;

}

/**
 * @brief add socket to multicast member ship
 *
 * @param fd  [in] socket fd
 * @param mrq [in] struct of multicast memver ship
 *
 * @return  0, if succ; -1, if failed.
 */
int add_socket_to_membership(int fd, struct ip_mreq *mrq)
{
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, \
                mrq, sizeof(struct ip_mreq)))
    {
        perror("setsockopt so_multicast_ttl");
        return -1;
    }

    return 0;

}

/**
 * @brief drop socket from multicast member ship
 *
 * @param fd  [in] socket fd
 * @param mrq [in] struct of multicast memver ship
 *
 * @return  0, if succ; -1, if failed.
 */
int drop_socket_from_membership(int fd, struct ip_mreq *mrq)
{
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, \
                mrq, sizeof(struct ip_mreq)))
    {
        perror("setsockopt so_multicast_ttl");
        return -1;
    }

    return 0;

}

/**
 * @brief get socket timeout of sending 
 *
 * @param fd  [in] socket fd
 *
 * @return send timeout, if succ; -1, if failed
 */
int get_socket_send_timeout(int fd)
{
    struct timeval tv = {0};
    //int tv = -1;
    int len = sizeof(tv);

    if (getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, \
                &tv, (socklen_t *)&len))
    {
        perror("getsockopt so_sndtimeo");
        return -1;
    }

    //return tv;
    return (tv.tv_sec * 1000 + tv.tv_usec);

}

/**
 * @brief get socket timeout of recving 
 *
 * @param fd  [in] socket fd
 *
 * @return recv timeout, if succ; -1, if failed
 */
int get_socket_recv_timeout(int fd)
{
    struct timeval tv = {0};
    int len = sizeof(tv);

    if (getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, \
                &tv, (socklen_t *)&len))
    {
        perror("getsockopt so_rcvtimeo");
        return -1;
    }

    return (tv.tv_sec * 1000 + tv.tv_usec);

}

/**
 * @brief set socket timeout of sending 
 *
 * @param fd     [in] socket fd
 * @param tm_ms  [in] timeout
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_send_timeout(int fd, int tm_ms)
{
    struct timeval tv = {0};
    int len = sizeof(tv);

    tv.tv_sec = tm_ms / 1000;
    tv.tv_usec = tm_ms % 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, \
                &tv, len))
    {
        perror("setsockopt so_sndtimeo");
        return -1;
    }

    return 0;

}

/**
 * @brief set socket timeout of recving 
 *
 * @param fd     [in] socket fd
 * @param tm_ms  [in] timeout
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_recv_timeout(int fd, int tm_ms)
{
    struct timeval tv = {0};
    int len = sizeof(tv);

    tv.tv_sec = tm_ms / 1000;
    tv.tv_usec = tm_ms % 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, \
                &tv, len))
    {
        perror("setsockopt so_rcvtimeo");
        return -1;
    }

    return 0;

}

/**
 * @brief get socket type 
 *
 * @param fd  [in] socket fd
 *
 * @return socket type, if succ; -1, if failed
 */
int get_socket_type(int fd)
{
    int type = -1;
    int len = sizeof(type);

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, \
                &type, (socklen_t *)&len))
    {
        perror("getsockopt so_sndtimeo");
        return -1;
    }

    return type;

}

/**
 * @brief get name of socket type
 *
 * @param fd [in] socket fd
 *
 * @return socket type's name, if succ; NULL, if failed.
 */
char* get_socket_type_str(int fd)
{
    int type = -1;
    int len = sizeof(type);
    int i = 0;

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, \
                &type, (socklen_t *)&len))
    {
        perror("getsockopt so_sndtimeo");
        return NULL;
    }
    
    while (socket_type[i].type_name != NULL)
    {
        if (socket_type[i].type_macro == type)
            break;
        i++;
    }

    return socket_type[i].type_name;
}

/**
 * @brief make network card hybrid mode
 *
 * @param ifname [in] interface name like eth0,eth2 and so on
 * @param fd     [in] socket fd
 * @param on     [in] hybrid mode swith
 *
 * @return 0, if succ; -1, if failed. 
 */
int make_socket_promisc(const char *ifname, int fd, int on)
{
    struct ifreq req;

    strcpy(req.ifr_name, ifname);
    if (ioctl(fd, SIOCGIFFLAGS, &req) < 0)
    {
        perror("ioctl get interface flags");
        return -1;
    }

    if (on) req.ifr_flags |= IFF_PROMISC;
    else req.ifr_flags &= ~IFF_PROMISC;

    if (ioctl(fd, SIOCSIFFLAGS, &req) < 0)
    {
        perror("ioctl set interface flags");
        return -1;
    }
    return 0;
}

/**
 * @brief get interface index 
 *
 * @param fd   [in] socket fd
 * @param req  [out] struct ifreq, return interface name
 *
 * @return 0, if succ; -1, if failed.
 */
int get_interface_index(int fd, struct ifreq *req)
{
    if (ioctl(fd, SIOCGIFINDEX, &req) < 0)
    {
        perror("get interface index");
        return -1;
    }

    return 0;
}

/**
 * @brief get interface name 
 *
 * @param ifname [out] interface name
 *
 * @return 0, if succ; -1, if failed.
 */
int get_ifname(char *ifname)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[2048];

    if (ifname == NULL) return -1;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        printf("socket error\n");
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) 
    {
        printf("ioctl error\n");
        return -1;
    }

    int count = 0;
    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    
    strcpy(ifname, "\0");
    for (; it != end; ++it) 
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) 
        {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) 
            { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) 
                {
                    count ++ ;
                    unsigned char * ptr ;
                    ptr = (unsigned char  *)&ifr.ifr_ifru.ifru_hwaddr.sa_data[0];
		            strcat(ifname, ifr.ifr_name);
                    strcat(ifname, " ");
                }
            }
        }
        else
        {
            printf("get mac info error\n");
            return -1;
        }
    }

    return 0;
}

/**
 * @brief detect little endian or big endian
 *
 * @return 1, if big endian; 0, if little endian; -1, if unkown.
 */
int is_big_endian()
{
    union {
        short s;
        char c[sizeof(short)];
    } un;

    un.s = 0x0102;
    if (sizeof(short) == 2)
    {
        if (un.c[0] == 0x01 && un.c[1] == 0x02)
        {
            printf("big endian\n");
            return 1;
        }
        else if (un.c[0] == 0x02 && un.c[1] == 0x01)
        {
            printf("little endian\n");
            return 0;
        }
        else 
        {
            printf("unknown\n");
            return -1;
        }
    }
    else printf("sizeof(short) = %d\n", sizeof(short));

    return -1;
}

/**
 * @brief ip is legal
 *
 * @param ip [in] ip address
 *
 * @return 1, if legal; 0, if illage
 */
int match_ip(const char *ip)
{
    int rt = 0;
    int point_count = 0;
    const char *p = ip;
    struct in_addr addr ;
    
    if (p == NULL) goto ret;
    while (*p != '\0')
    {
        if (*p == '.') point_count++;
        p++;
    }
    if (point_count != 3 || (p - ip) < 7) goto ret;

    if (!inet_aton(ip, &addr)) goto ret;
    rt = inet_aton(inet_ntoa(addr), NULL);

ret:
    return rt;
}

/*************************************************************
********  Function Declaration Of Socket Application  ********
**************************************************************/
/**
 * @brief create a server
 *
 * @param sck      [in] tcp sock
 * @param domain   [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param type     [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param port     [in] listen port
 *
 * @return 0, if succ; -1, if failed.
 */
int server_create(struct socket *sck, int domain, \
                int type, u_short port, const char *ip)
{
    thread_runtine thread = NULL;
    if (sck == NULL || sck->fd > 0) return -1;

    /* remove socket file when socket type is local socket */
    if (domain == AF_UNIX || domain == AF_LOCAL)
    {
        if (ip != NULL && !access(ip, F_OK))
            unlink(ip);
    }

    /* create, bind and listen socket */
    sck->fd = socket_startup(domain, type, &sck->addr, port, ip, 1);

    /* set socket block */
    make_socket_block(sck->fd);
   
    if (type == SOCK_DGRAM) thread = udp_backup_service;
    else thread = tcp_server_backup_service;

    /* create tcp server thread runtine */
    if (pthread_create(&sck->ptd, NULL, thread, sck))
    {
        printf("tcp server create failed.\n");
        return -1;
    }

    return 0;
}

/**
 * @brief stop tcp server
 *
 * @param sck [in] sock
 */
void server_stop(struct socket *sck)
{
    pthread_cancel(sck->ptd);
    
    return;
}

/**
 * @brief connect to a server
 *
 * @param sck  [in] client tcp sock
 * @param domain [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param ip   [in] tcp server ip address
 * @param port [in] tcp server port
 *
 * @return 0, if succ; -1, if fail 
 */
int client_connect(struct socket *sck, int domain, \
                int type, const char *ip, u_short port)
{
    struct sockaddr_un addr_un = {0};
    thread_runtine thread = NULL;
    void *addr = NULL;

    if (sck->fd > 0) return -1;

    /* set up address struct according to socket type */
    if (domain == AF_UNIX || domain == AF_LOCAL) 
        addr = &addr_un;
    else addr = &sck->addr;

    /* create, bind and listen socket */
    sck->fd = socket_startup(domain, type, addr, port, ip, 0);

    /* set socket block */
    make_socket_block(sck->fd);

    /* function pointer of thread */
    if (type == SOCK_DGRAM) 
    {
        thread = udp_backup_service;
    }
    else 
    {
        thread = tcp_client_backup_service;

        /* connect server */
        if (!socket_connect(sck->fd, (struct sockaddr *)addr))
        {
            socket_event_process(sck->fd, sck->evl.on_connect);
        }
        else perror("connect");
    }

    if (pthread_create(&sck->ptd, NULL, thread, sck))
    {
        printf("client_connect pthread create failed.\n");
        return -1;
    }

    return 0;
}


/**
 * @brief connect to a server
 *
 * @param sck   [in] client tcp sock
 * @param domain [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param ip    [in] tcp server ip address
 * @param port  [in] tcp server port
 * @param tm_ms [in] connect timeout
 *
 * @return 0, if succ; -1, if fail
 */
int client_time_connect(struct socket *sck, int domain, \
            int type, const char *ip, u_short port, int tm_ms)
{
    struct sockaddr_un addr_un = {0};
    thread_runtine thread = NULL;
    void *addr = NULL;

    if (sck->fd > 0) return -1;

    /* set up address struct according to socket type */
    if (domain == AF_UNIX || domain == AF_LOCAL) 
        addr = &addr_un;
    else addr = &sck->addr;

    /* create, bind and listen socket */
    sck->fd = socket_startup(domain, type, addr, port, ip, 0);

    
    /* set socket block */
    make_socket_block(sck->fd);

    /* function pointer of thread */
    if (type == SOCK_DGRAM) 
    {
        thread = udp_backup_service;
    }
    else 
    {
        thread = tcp_client_backup_service;

        /* connect server */
        if (!socket_time_connect(sck->fd, (struct sockaddr *)addr, tm_ms))
        {
            socket_event_process(sck->fd, sck->evl.on_connect);
        }
        else perror("connect");
    }

    if (pthread_create(&sck->ptd, NULL, thread, sck))
    {
        printf("client_connect pthread create failed.\n");
        return -1;
    }

    return 0;
}


/*************************************************************
**  Function Declaration Of TCP Socket Background Service  ***
**************************************************************/

/**
 * @brief tcp_server thread runine
 *
 * @param sock [in] struct sock
 *
 * @return 
 */
static  void* tcp_server_backup_service(void *sock)
{
    /* define */
    struct socket *sck = (struct socket *)sock;
    struct event_loop *evl = (event_loop_t *)&sck->evl;
    struct timeval tv = {0};
    fd_set set;
    fd_set allset;
    int can_recv_bytes = 0;
    int nready = 0;
    int serfd = sck->fd;
    int maxfd = sck->fd;
    int clifd = -1;
    int fd = -1;
    int maxi = -1;
    int i = 0;


    /* avoid sock equal to NULL */
    if (sock == NULL) return NULL;

    /* client socket fd init */
    for (i = 0; i < MAX_CLIENT_NUM; i++)
        sck->cli_fd[i] = -1;

    /* empty fd sets */
    FD_ZERO(&allset);
    FD_SET(serfd, &allset);
    
    /* detect socket state looply */
    while (1)
    {
        /* set timer */
        tv.tv_sec = 0;
        tv.tv_usec = 10000;

        /* set fd sets */
        set = allset;
        
        /* time waiting */
        nready = select(maxfd + 1, &set, NULL, NULL, &tv);

        /* detect server socket state */
        if (FD_ISSET(serfd, &set))
        {
            /* accept socket */
            clifd = socket_accept(serfd);
            
            /* on_accept */
            if (clifd > 0)
                socket_event_process(clifd, evl->on_accept);

            for (i = 0; i < MAX_CLIENT_NUM; i++)
            {
                if (sck->cli_fd[i] < 0) 
                {
                    sck->cli_fd[i] = clifd;
                    break;
                }
            }

            if (i == MAX_CLIENT_NUM)
                printf("too many client!\n");
            
            /* add socket fd to the collection */
            FD_SET(clifd, &allset);
            
            /* set maxfd and maxi*/
            if (clifd > maxfd) maxfd = clifd;
            if (i > maxi) maxi = i;

            if (--nready <= 0) continue;
        }

        /* detect client socket state */
        for (i = 0; i <= maxi; i++)
        {
            if ((fd = sck->cli_fd[i]) < 0) continue;

            if (FD_ISSET(fd, &set))
            {
                /* on_recv */
                can_recv_bytes = get_can_read_bytes(fd);
                if (can_recv_bytes > 0)
                    socket_event_process(fd, evl->on_recv);

                /* on_close */
                if (recv(fd, NULL, 0, 0) == 0)
                {
                    socket_event_process(fd, evl->on_close);

                    close(fd);
                    FD_CLR(fd, &allset);
                    sck->cli_fd[i] = -1;
                }

                if (--nready <= 0) break;
            }
        }
    }

    return NULL;
}


/**
 * @brief tcp_server thread runine
 *
 * @param sock [in] struct sock
 *
 * @return 
 */
static void* tcp_client_backup_service(void *sock)
{
    /* define */
    struct socket *sck = (struct socket *)sock;
    struct event_loop *evl = (event_loop_t *)&sck->evl;
    struct timeval tv = {0};
    fd_set set;
    fd_set allset;
    int fd = sck->fd;
    int maxfd = sck->fd;
    int can_recv_bytes = 0;
    int nready = 0;

    /* avoid sock equal to NULL */
    if (sock == NULL) return NULL;

    /* empty fd sets */
    FD_ZERO(&allset);
    FD_SET(fd, &allset);
    
    /* detect socket state looply */
    while (1)
    {
        /* set timer */
        tv.tv_sec = 0;
        tv.tv_usec = 10000;

        /* set fd sets */
        set = allset;
        
        /* time waiting */
        nready = select(maxfd + 1, &set, NULL, NULL, &tv);

        if (FD_ISSET(fd, &set))
        {
            /* on_recv */
            can_recv_bytes = get_can_read_bytes(fd);
            if (can_recv_bytes > 0)
                socket_event_process(fd, evl->on_recv);

            /* on_close */
            if (recv(fd, NULL, 0, 0) == 0)
            {
                socket_event_process(fd, evl->on_close);

                close(fd);
                FD_CLR(fd, &allset);
            }
        }
    }

    return NULL;
}


/*************************************************************
**  Function Declaration Of UDP Socket Background Service  ***
**************************************************************/
/**
 * @brief tcp_server thread runine
 *
 * @param sock [in] struct sock
 *
 * @return 
 */
static void* udp_backup_service(void *sock)
{
    /* define */
    struct socket *sck = (struct socket *)sock;
    struct event_loop *evl = (event_loop_t *)&sck->evl;
    struct timeval tv = {0};
    fd_set set;
    fd_set allset;
    int fd = sck->fd;
    int maxfd = sck->fd;
    int nready = 0;
    int can_recv_bytes = 0;

    /* avoid sock equal to NULL */
    if (sock == NULL) return NULL;

    /* empty fd sets */
    FD_ZERO(&allset);
    FD_SET(fd, &allset);
    
    /* detect socket state looply */
    while (1)
    {
        /* set timer */
        tv.tv_sec = 0;
        tv.tv_usec = 10000;

        /* set fd sets */
        set = allset;
        
        /* time waiting */
        nready = select(maxfd + 1, &set, NULL, NULL, &tv);

        /* on_recv */
        if (FD_ISSET(fd, &set))
        {
            can_recv_bytes = get_can_read_bytes(fd);
            if (can_recv_bytes > 0)
                socket_event_process(fd, evl->on_recv);

            if (--nready <= 0) continue;
        }

    }

    return NULL;
}


/*************************************************************
********  Function Declaration Of Socket Broadcast  **********
**************************************************************/
/**
 * @brief udp broadcast send data
 *
 * @param cast_ip     [in]  ip address of broadcast
 * @param port        [in]  port of broadcast
 * @param cast_times  [in]  broadcast times
 * @param cast_info   [in] broadcast data
 *
 * @return 0, if succ; -1, if failed.
 */
int udp_broadcast_send(const char *cast_ip, int port, int cast_times,\
        const char *cast_info)
{
    int fd = -1;
    struct sockaddr_in addr = {0};

    if (cast_times < 1) return -1;

    /**
     * 1. socket create and init
     * 2. set socket broadcast
     */
    fd = socket_create(AF_INET, SOCK_DGRAM);
    socket_addr_init(&addr, AF_INET, port, cast_ip);
    make_socket_broadcast(fd, 1);

    /**
     * send broadcast data cycly
     */
    while (cast_times-- > 0)
    {
        socket_addr_sendto(fd, (void *)cast_info, strlen(cast_info), &addr);
        sleep(1);
    }

    socket_close(fd);

    return 0;
}

/**
 * @brief udp broadcast recv data
 *
 * @param cast_ip     [in]  ip address of broadcast
 * @param port        [in]  port of broadcast
 * @param cast_times  [in]  broadcast times
 * @param cast_info   [out] broadcast data
 * @param size        [in]  size of broadcast data
 *
 * @return 0, if succ; -1, if failed.
 */
int udp_broadcast_recv(const char *cast_ip, int port, int cast_times,\
        char *cast_info, int size)
{
    int fd = -1;
    struct sockaddr_in addr = {0};

    if (cast_times < 1) return -1;

    /**
     * 1. socket create and init
     * 2. set socket broadcast
     * 3. bind socket 
     */
    fd = socket_create(AF_INET, SOCK_DGRAM);
    socket_addr_init(&addr, AF_INET, port, cast_ip);
    make_socket_broadcast(fd, 1);
    socket_bind(fd, &addr);

    /**
     * recv broadcast data cycly
     */
    while (cast_times-- > 0)
    {
        if (socket_addr_recvfrom(fd, cast_info, \
                size, &addr) > 0)
            printf("broadcast recv from %s: %s\n", \
                inet_ntoa(addr.sin_addr), cast_info);
    }

    socket_close(fd);

    return 0;
}


        
/*************************************************************
********  Function Declaration Of Socket Multicast  **********
**************************************************************/
/**
 * @brief udp multicast send data
 *
 * @param cast_ip     [in]  ip address of broadcast
 * @param port        [in]  port of broadcast
 * @param cast_times  [in]  broadcast times
 * @param cast_info   [in] broadcast data
 *
 * @return 0, if succ; -1, if failed.
 */
int udp_multicast_send(const char *cast_ip, int port, int cast_times,\
        const char *cast_info)
{
    int fd = -1;
    struct sockaddr_in addr = {0};

    if (cast_times < 1) return -1;

    /**
     * socket create and init
     */
    fd = socket_create(AF_INET, SOCK_DGRAM);
    socket_addr_init(&addr, AF_INET, port, cast_ip);

    /**
     * send multicast data cycly
     */
    while (cast_times-- > 0)
    {
        socket_addr_sendto(fd, (void *)cast_info, strlen(cast_info), &addr);
        sleep(1);
    }

    socket_close(fd);

    return 0;
}

/**
 * @brief udp multicast recv data
 *
 * @param cast_ip     [in]  ip address of broadcast
 * @param port        [in]  port of broadcast
 * @param cast_times  [in]  broadcast times
 * @param cast_info   [out] broadcast data
 * @param size        [in]  size of broadcast data
 *
 * @return 0, if succ; -1, if failed.
 */
int udp_multicast_recv(const char *cast_ip, int port, int cast_times,\
        char *cast_info, int size)
{
    int fd = -1;
    struct sockaddr_in addr = {0};
    struct ip_mreq mreq ;

    if (cast_times < 1) return -1;
    
    /**
     * 1. socket create and init
     * 2. bind socket
     */
    fd = socket_create(AF_INET, SOCK_DGRAM);
    socket_addr_init(&addr, AF_INET, port, cast_ip);
    make_listen_socket_reuseable(fd);
    socket_bind(fd, &addr);

    /**
     * 1. set socket multicast ttl
     * 2. set socket multicast loop
     */
    make_socket_multicast_ttl(fd, 5);
    make_socket_multicast_loop(fd, 1);
    
    /**
     * add socket to multicast member ship
     */
    mreq.imr_multiaddr.s_addr = inet_addr(cast_ip);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    add_socket_to_membership(fd, &mreq);

    /**
     * recv multicast data cycly
     */
    while (cast_times-- > 0)
    {
        if (socket_addr_recvfrom(fd, cast_info, \
                size, &addr) > 0)
            printf("multicast recv from %s: %s\n", \
                inet_ntoa(addr.sin_addr), cast_info);
    }

    /**
     * drop socket from multicast member ship
     */
    drop_socket_from_membership(fd, &mreq);
    socket_close(fd);

    return 0;
}



/*************************************************************
*******  Function Declaration Of Socket Data Capture  ********
**************************************************************/
/**
 * @brief ethdump socket init 
 *
 * @return socket fd, if succ; -1, if failed.
 */
int ethdump_socket_init()
{
    struct sockaddr_ll addrll = {0};
    char ifname[20] = {0};
    char *p = ifname;
    int fd = -1;

    /**
     * create raw socket used to recv all data from interface
     */
    if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        perror("raw socket create");
        return -1;
    }

    /**
     * get interface name
     */
    get_ifname(p);

    /**
     * make interface working on hybrid mode
     */
    make_socket_promisc(ifname, fd, 1);

    /**
     * set recv buf size 
     */
    if (set_socket_recv_buf(fd, 5 * 1024) < 0) return -1;

    /**
     * socket addr init
     */
    addrll.sll_family = PF_PACKET;
    addrll.sll_ifindex = 0;
    addrll.sll_protocol = htons(ETH_P_ALL);

    /**
     * socket bind used to recvfrom data
     */
    if (bind(fd, (struct sockaddr *)&addrll, sizeof(addrll)) < 0)
    {
        perror("bing the interface");
        close(fd);
        return -1;
    }

    return fd;
}

/**
 * @brief print address of mac 
 *
 * @param type   [in] type of mac, like src mac or dst mac
 * @param addr[] [in] address of mac
 */
void show_mac(int type, const unsigned char addr[])
{
    int i = 0;

    if (addr == NULL) return;
    
    if (type == 0) printf("SRC MAC = [");
    else printf("DST MAC = [");
    
    for (i = 0; i < ETHER_ADDR_LEN - 1; i++)
        printf("%02x:", (unsigned char)addr[i]);
    printf("%02x] ", (unsigned char)addr[i]);

    return;
}

/**
 * @brief parse ether header
 *
 * @param machead  [in]  head of mac address
 * @param eth_type [out] type of ether, like 0x0800 -- ip
 * @param src_mac  [out] mac address of src
 * @param dst_mac  [out] mac address of dst
 *
 * @return 0, if succ; -1, if failed.
 */
int parse_eth_head(const struct ether_header *machead, \
        unsigned short *eth_type, char **src_mac, \
        char **dst_mac)
{
    int i = 0;

    if (machead == NULL) return -1;

    if (eth_type != NULL)
        *eth_type = ntohs(machead->ether_type);

    if (src_mac != NULL)
    {
        for (i = 0; i < ETHER_ADDR_LEN - 1; i++)
            sprintf(*src_mac, "%s%02x:", *src_mac, machead->ether_shost[i]);
        sprintf(*src_mac, "%s%02x", *src_mac, machead->ether_shost[i]);
    }

    if (dst_mac != NULL)
    {
        for (i = 0; i < ETHER_ADDR_LEN - 1; i++)
            sprintf(*dst_mac, "%s%02x:", *dst_mac, machead->ether_dhost[i]);
        sprintf(*dst_mac, "%s%02x", *dst_mac, machead->ether_dhost[i]);
    }

    return 0;
}

/**
 * @brief get src ip from ip address header
 *
 * @param iphead  [in]  ip address header
 * @param src_ip  [out] ip address of src
 *
 * @return ip address of src, if succ; NULL, if failed.
 */
char* get_src_ip(const struct ip *iphead, char **src_ip)
{
    if (iphead == NULL) return 0;

    if (src_ip != NULL)
    {
        strcpy(*src_ip, inet_ntoa(iphead->ip_src));
        return *src_ip;
    }
    else return inet_ntoa(iphead->ip_src);
}

/**
 * @brief get dst ip from ip address header
 *
 * @param iphead  [in]  ip address header
 * @param dst_ip  [out] ip address of dst
 *
 * @return ip address of src, if succ; NULL, if failed.
 */
char* get_dst_ip(const struct ip *iphead, char **dst_ip)
{
    if (iphead == NULL) return 0;

    if (dst_ip != NULL)
    {
        strcpy(*dst_ip, inet_ntoa(iphead->ip_dst));
        return *dst_ip;
    }
    else return inet_ntoa(iphead->ip_dst);
}

/**
 * @brief get protocol name, like tcp, udp and so on 
 *
 * @param iphead     [in]  ip address header
 * @param proto_name [out] protocol name
 *
 * @return protocol name, if succ; NULL, if failed.
 */
char* get_proto_name(const struct ip *iphead, char **proto_name)
{
    struct protoent *proto = NULL;

    proto = getprotobynumber(iphead->ip_p);

    if (proto_name != NULL)
    {
        strcpy(*proto_name, proto->p_name);
        return *proto_name;
    }
    else return NULL;
}

/**
 * @brief parse ip header 
 *
 * @param iphead     [in]  ip address header
 * @param proto_name [out] protocol name
 * @param ip_src     [out] ip address of src
 * @param ip_dst     [out] ip address of dst
 *
 * @return 0, if succ; -1, if failed.
 */
int parse_ip_head(const struct ip *iphead, \
        char **proto_name, char **ip_src, char **ip_dst)
{
    if (iphead == NULL) return -1;

    get_proto_name(iphead, proto_name);
    get_src_ip(iphead, ip_src);
    get_dst_ip(iphead, ip_dst);

    return 0;
}

/**
 * @brief parse tcp header 
 * 
 * @param tcphead [in] tcp header
 *
 * @return 0, if succ; -1, if failed.
 */
int parse_tcp_head(const struct tcphdr *tcphead)
{
    if (tcphead == NULL) return -1;

    printf("TCP: \n");
    printf("    src port: %d\n", ntohs(tcphead->source));
    printf("    dst port: %d\n", ntohs(tcphead->dest));
    //printf("TCP DATA: %s\n", (char *)(tcphead + 1));

    return 0;
}

/**
 * @brief parse udp header 
 * 
 * @param udphead [in] udp header
 *
 * @return 0, if succ; -1, if failed.
 */
int parse_udp_head(const struct udphdr *udphead)
{
    if (udphead == NULL) return -1;

    printf("UDP: \n");
    printf("    src port: %d\n", ntohs(udphead->source));
    printf("    dst port: %d\n", ntohs(udphead->dest));
    printf("    data len: %d\n", ntohs(udphead->len));
    //printf("UDP DATA: %d ", ntohs(udphead->len));
    //printf("UDP DATA: %s\n", (char *)(udphead + 1));

    return 0;
}

/**
 * @brief parse ether frame 
 *
 * @param data [in] ether frame data
 * 
 * @return 0, if succ; if failed
 */
int parse_ethframe(const char *data)
{
    struct ether_header *ethh = NULL;
    struct ip *iph = NULL;
    //struct tcphdr *tcph = NULL;
    struct udphdr *udph = NULL;
    unsigned short eth_type = 0;

    if (data == NULL) return -1;

    ethh = (struct ether_header *)data;
    parse_eth_head(ethh, &eth_type, NULL, NULL);
    if (eth_type != 0x0800) return -1;

    iph = (struct ip *)(ethh + 1);
    //if (iph->ip_p != 1 && iph->ip_p != 2) return -1;
    parse_ip_head(iph, NULL, NULL, NULL);

    /*if (iph->ip_p == 6)
    {
        tcph = (struct tcphdr *)(iph + 1);
        parse_tcp_head(tcph);
    }*/

    if(iph->ip_p == 17)
    {
        udph = (struct udphdr *)(iph + 1);
        //printf("udp: src port[%d], dst port[%d]\n", 
        //        ntohs(udph->source), ntohs(udph->dest));
        if (ntohs(udph->dest) == 5001)
        {
            parse_udp_head(udph);
            //printf("\n");

            int len =  ntohs(udph->len);
            char buf[128] = {0};
            udph++;

            strncpy(buf, (char *)udph, len - 8);
            printf("    data: %s\n", buf);
        }
    }

    //printf("\n");

    return 0;
}

/**
 * @brief capture ether data 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if failed.
 */
int eth_data_capture(int fd)
{
    char buf[ 5 * 1024] = {0};
    socklen_t len = 0;

    while (1)
    {
        if (recvfrom(fd, buf, sizeof(buf), 0, NULL, &len) > 0)
            parse_ethframe(buf);
    }

    return 0;
}

/**
 * @brief start data capture 
 *
 * @return 0, if succ; -1, if failed.
 */
int start_data_capture()
{
    int fd = -1;

    if ((fd = ethdump_socket_init()) < 0) return -1;
    eth_data_capture(fd);
    close(fd);

    return 0;
}
