#include "socket_base.h"
#define ipc_path "./ipc"

int main(int argc, char *argv[])
{
    int fd = -1;
    int clifd = -1;
    int len = 0;
    char buf[128] = {0};

    if ((fd = local_socket_startup(SOCK_STREAM, NULL, ipc_path, 1)) == -1)
        return -1;

    clifd = socket_accept(fd);
    printf("connect succ\n");
    while ((len = socket_recv(clifd, buf, sizeof(buf))))
    {
        printf("len: %d, info: %s\n", len, buf);

        socket_send(clifd, "hi", 3);
    }

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
 * @brief init inet sockaddr_in
 *
 * @param addr   [in] sockaddr_in
 * @param ip     [in] ip address
 * @param port   [in] port
 */
void inet_addr_init(struct sockaddr_in *addr, const char *ip, u_short port)
{
    /**
     * init struct of socket address
     */
    addr->sin_family = AF_INET;
    if (ip == NULL) addr->sin_addr.s_addr = htonl(INADDR_ANY);
    else addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(port);

    return;
}

/**
 * @brief init locak sockaddr_un
 *
 * @param addr   [in] sockaddr_un
 * @param path   [in] path
 */
void local_addr_init(struct sockaddr_un *addr, const char *path)
{

    /* exit if ip is NULL */
    if (path == NULL) 
    {
        printf("AF_UNIX path can't be NULL.\n");
        exit(1);
    }

    /**
     * init struct of socket address
     */
    memset(addr, 0, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strncpy(addr->sun_path, path, sizeof(addr->sun_path));
    addr->sun_path[sizeof(addr->sun_path) - 1] = '\0';
    //unlink(addr->sun_path) ;

    return;
}

/**
 * @brief bind a socket¡£
 *
 * @param addr [in]
 * @param fd   [in] fd of server socket¡£
 *
 * @return 0, if succ; -1, if failed.
 */
int socket_bind(int fd, struct sockaddr *addr)
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
 * @brief start up a internet server socket
 *
 * @param domain [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param port   [in] socket port
 * @param ip     [in] ip address
 * @param is_ser [in] srever or client
 *
 * @return socket fd, if succ; exit, if fail
 */
int inet_socket_startup(int type, struct sockaddr_in *addr, const char *ip, u_short port, int is_ser)
{
    int fd = -1;
    struct sockaddr_in in_addr = {0};

    if (is_ser && addr == NULL) addr = &in_addr;

    /* create socket */
    fd = socket_create(AF_INET, type);
    make_listen_socket_reuseable(fd);
    
    /* init sockaddr_in */
    if (!is_ser && addr == NULL)
    {
        fprintf(stderr, "%s", "struct sockaddr_in cannot be NULL\n");
        exit(1);
    }
    inet_addr_init(addr, ip, port);
    
    /* bind socket */
    if (is_ser) 
    {
        socket_bind(fd, (struct sockaddr *)addr);
    
        /* build socket listen */
        if (type == SOCK_STREAM) socket_listen(fd, 5);
    }

    return fd;
}

/**
 * @brief start up a local server socket
 *
 * @param domain [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param path   [in] socket file path
 * @param is_ser [in] srever or client
 *
 * @return socket fd, if succ; exit, if fail
 */
int local_socket_startup(int type, struct sockaddr_un *addr, const char *path, int is_ser)
{
    int fd = -1;
    struct sockaddr_un un_addr = {0};

    /* create socket */
    fd = socket_create(AF_UNIX, type);
    make_listen_socket_reuseable(fd);
    
    /* init sockaddr_in */
    if (is_ser && addr == NULL) addr = &un_addr;
    local_addr_init(addr, path);
    
    /* bind and listen socket */
    if (is_ser) 
    {
        unlink(addr->sun_path);
        socket_bind(fd, (struct sockaddr *)&addr);
        if (type == SOCK_STREAM) socket_listen(fd, 5);
    }
    
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


/*************************************************************
*********  Function Declaration Of UDP Socket Recv  **********
**************************************************************/
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
 * @brief get hardware address by interface name
 *
 * @param ifname [in]  interface name
 * @param mac    [out] hardware addree
 *
 * @return 0, if succ; -1, if failed
 */
int get_mac_addr(const char *ifname, char *mac)
{
    int fd = -1;
    struct ifreq ifreq;

    if (mac == NULL) return -1;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    memset(&ifreq, 0, sizeof(ifreq));
    strcpy(ifreq.ifr_name, ifname);

    if (ioctl(fd, SIOCGIFHWADDR, &ifreq) < 0)
        return -1;
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
            (unsigned char) ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[5]
            );

    return 0;
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
