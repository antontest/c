#include "socket_app.h"

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
    inet_addr_init(&addr, cast_ip, port);
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
    inet_addr_init(&addr, cast_ip, port);
    make_socket_broadcast(fd, 1);
    socket_bind(fd, (struct sockaddr *)&addr);

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
    inet_addr_init(&addr, cast_ip, port);

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
    inet_addr_init(&addr, cast_ip, port);
    make_listen_socket_reuseable(fd);
    socket_bind(fd, (struct sockaddr *)&addr);

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
    struct socket_impl *sck = (struct socket_impl *)sock;
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
    int addr_len = sizeof(struct sockaddr);

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
            //clifd = socket_accept(serfd);
            clifd = accept(serfd, \
                    (struct sockaddr *)&sck->addr.in_addr, 
                    (socklen_t *)&addr_len);
            
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
                if (can_recv_bytes > 0 || get_socket_protocol(fd) == IPPROTO_SCTP)
                {
                    socket_event_process(fd, evl->on_recv);
                    sck->curr_cli_fd = fd;
                }
                
                /* on_close */
                if (can_recv_bytes <= 0)
                {
                    FD_CLR(fd, &allset);
                    socket_event_process(fd, evl->on_close);

                    if (fd > 0)close(fd);
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
    struct socket_impl *sck = (struct socket_impl *)sock;
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
    FD_SET(sck->fd, &allset);
    
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

        if (FD_ISSET(sck->fd, &set))
        {
            /* on_recv */
            can_recv_bytes = get_can_read_bytes(fd);
            if (can_recv_bytes > 0)
                socket_event_process(fd, evl->on_recv);
            /* on_close */
            else if (can_recv_bytes <= 0)
            {
                FD_CLR(fd, &allset);
                socket_event_process(fd, evl->on_close);
                if (fd > 0) close(fd);
                fd = -1;
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
    struct socket_impl *sck = (struct socket_impl *)sock;
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
int inet_server_create(struct socket_impl *sck, int type, const char *ip, u_short port)
{
    thread_runtine thread = NULL;
    if (sck == NULL || sck->fd > 0) return -1;

    /* init impl */
    memset(sck, 0, sizeof(*sck));
    
    /* create, bind and listen socket */
    sck->fd = startup_inet_server(type, ip, port);

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
 * @brief create a local server
 *
 * @param sck      [in] tcp sock
 * @param domain   [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param type     [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param port     [in] listen port
 *
 * @return 0, if succ; -1, if failed.
 */
int local_server_create(struct socket_impl *sck, int type, const char *path)
{
    thread_runtine thread = NULL;
    if (sck == NULL || sck->fd > 0) return -1;

    /* init impl */
    memset(sck, 0, sizeof(*sck));
    
    /* remove socket file when socket type is local socket */
    if (path != NULL && !access(path, F_OK))
        unlink(path);

    /* create, bind and listen socket */
    sck->fd = startup_local_server(type, path);

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
void server_stop(struct socket_impl *sck)
{
    pthread_cancel(sck->ptd);
    
    return;
}

/**
 * @brief connect to a server
 *
 * @param sck  [in] client tcp sock
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param ip   [in] tcp server ip address
 * @param port [in] tcp server port
 *
 * @return 0, if succ; -1, if fail 
 */
int inet_client_connect(struct socket_impl *sck, \
                int type, const char *ip, u_short port)
{
    thread_runtine thread = NULL;

    if (sck == NULL || sck->fd > 0) return -1;

    /* init impl */
    //memset(sck, 0, sizeof(*sck));
    
    /* create, bind and listen socket */
    sck->fd = startup_inet_client(type, (struct sockaddr_in *)&sck->addr.in_addr, ip, port);

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
        if (!socket_connect(sck->fd, (struct sockaddr *)&sck->addr.in_addr))
        {
            socket_event_process(sck->fd, sck->evl.on_connect);
        }
        else
        {
            printf("connect failed\n");
            return -1;
        }
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
 * @param sck  [in] client tcp sock
 * @param type [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param path [in] tcp server ip address
 *
 * @return 0, if succ; -1, if fail 
 */
int local_client_connect(struct socket_impl *sck, int type, const char *path)
{
    thread_runtine thread = NULL;

    if (sck == NULL || sck->fd > 0) return -1;

    /* init impl */
    //memset(sck, 0, sizeof(*sck));
    
    /* create, bind and listen socket */
    sck->fd = startup_local_client(type, (struct sockaddr_un *)&sck->addr.un_addr, path);

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
        if (!socket_connect(sck->fd, (struct sockaddr *)&sck->addr.un_addr))
        {
            socket_event_process(sck->fd, sck->evl.on_connect);
        }
        else
        {
            printf("connect failed\n");
            return -1;
        }
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
 * @param type  [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param ip    [in] tcp server ip address
 * @param port  [in] tcp server port
 * @param tm_ms [in] connect timeout
 *
 * @return 0, if succ; -1, if fail
 */
int inet_client_time_connect(struct socket_impl *sck, int type, \
                            const char *ip, u_short port, int tm_ms)
{
    thread_runtine thread = NULL;

    if (sck == NULL || sck->fd > 0) return -1;

    /* init impl */
    //memset(sck, 0, sizeof(*sck));
    
    /* create, bind and listen socket */
    sck->fd = startup_inet_client(type, (struct sockaddr_in *)&sck->addr.in_addr, ip, port);

    
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
        if (!socket_time_connect(sck->fd, (struct sockaddr *)&sck->addr.in_addr, tm_ms))
        {
            socket_event_process(sck->fd, sck->evl.on_connect);
        }
        else
        {
            printf("connect failed\n");
            return -1;
        }
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
 * @param type  [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param path  [in] tcp server ip address
 * @param port  [in] tcp server port
 * @param tm_ms [in] connect timeout
 *
 * @return 0, if succ; -1, if fail
 */
int local_client_time_connect(struct socket_impl *sck, int type, \
                                const char *path, int tm_ms)
{
    thread_runtine thread = NULL;

    if (sck == NULL || sck->fd > 0) return -1;

    /* init impl */
    //memset(sck, 0, sizeof(*sck));
    
    /* create, bind and listen socket */
    sck->fd = startup_local_client(type, (struct sockaddr_un *)&sck->addr.un_addr, path);

    
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
        if (!socket_time_connect(sck->fd, (struct sockaddr *)&sck->addr.un_addr, tm_ms))
        {
            socket_event_process(sck->fd, sck->evl.on_connect);
        }
        else
        {
            printf("connect failed\n");
            return -1;
        }
    }

    if (pthread_create(&sck->ptd, NULL, thread, sck))
    {
        printf("client_connect pthread create failed.\n");
        return -1;
    }

    return 0;
}


/*************************************************************
***********  Function Declaration Of Socket RAW  *************
**************************************************************/
/**
 * @brief ethdump socket init 
 *
 * @return socket fd, if succ; -1, if failed.
 */
int raw_socket_init()
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
    //make_socket_promisc(ifname, fd, 1);

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

int parse_ether_head(void *data, unsigned char dhost[6], \
        unsigned char shost[6], unsigned short *proto_type)
{
    int i = 0;
    struct ether_hdr *mac_hd = NULL;
    
    if (data == NULL) return -1;
    
    mac_hd = (struct ether_hdr *)data;
    if (dhost != NULL)
    {
        while (i < 6)
        {
            dhost[i] = mac_hd->ether_dhost[i];
            i++;
        }
    }
    
    if (shost != NULL)
    {
        i = 0;
        while (i < 6)
        {
            shost[i] = mac_hd->ether_shost[i];
            i++;
        }
    }
    
    if (proto_type != NULL)
        *proto_type = ntohs(mac_hd->proto_type);
    
    return 0;
}

void print_mac(const char *info, unsigned char mac_addr[6])
{
    int i = 0;
    
    if (info != NULL) printf("%s: ", info);
    
    while (i < 5)
        printf("%02x-", mac_addr[i++]);
    printf("%02x\n", mac_addr[i]);
    
    return ;
}

int parse_icmp_head(void *data, unsigned char *type, unsigned char *code)
{
    struct ether_hdr *mac_hd = NULL;
    struct icmp_hdr *icmp_hd = NULL;
    
    if (data == NULL) return -1;
    
    mac_hd = (struct ether_hdr *)data;
    icmp_hd = (struct icmp_hdr *)(mac_hd + 1);
    
    if (type != NULL)
        *type = ntohs(icmp_hd->i_type);
    
    if (code != NULL)
        *code = ntohs(icmp_hd->i_code);
    
    return 0;
}

int parse_ping(void *data)
{
    struct ether_hdr *mac_hd = NULL;
    struct icmp_hdr *icmp_hd = NULL;
    struct ping_hdr *ping_hd = NULL;
    
    if (data == NULL) return -1;
    
    mac_hd = (struct ether_hdr *)data;
    icmp_hd = (struct icmp_hdr *)(mac_hd + 1);
    ping_hd = (struct ping_hdr *)(icmp_hd + 1);
    
    printf("ping: ");
    printf("%s\n", (char *)ping_hd);
    
    return 0;
}

/*************************************************************
***********  Function Declaration Of Socket SCTP  ************
**************************************************************/
/**
 * @brief sctp_server_create 
 *
 * @param sck  [in] socket impl
 * @param ip   [in] listen ip
 * @param port [in] listen port
 *
 * @return 0, if succ; -1, if failed
 */
int sctp_server_create(struct socket_impl *sck, const char *ip, unsigned short port)
{
    struct sctp_initmsg initmsg = {0};
    struct sctp_event_subscribe event = {0};
    if (sck == NULL || sck->fd > 0) return -1;

    /* init impl */
    memset(sck, 0, sizeof(*sck));
    
    /* create, bind and listen socket */
    if ((sck->fd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0)
    {
        perror("sctp socket failed");
    }

    /* socket bind */
    inet_addr_init(&sck->addr.in_addr, ip, port);
    socket_bind(sck->fd, (struct sockaddr *)&sck->addr.addr);

    /* init msg */
    initmsg.sinit_num_ostreams = 5;
    initmsg.sinit_max_instreams = 5;
    initmsg.sinit_max_attempts = 4;
    if (setsockopt(sck->fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg)) < 0)
        perror("sctp setsockopt initmsg failed");

    /* sctp event */
    event.sctp_data_io_event = 1;
    if (setsockopt(sck->fd, IPPROTO_SCTP, SCTP_EVENTS, &event, sizeof(event)) < 0)
        perror("sctp setsockopt event failed");

    /* socket listen */
    socket_listen(sck->fd, 5);

    /* set socket block */
    make_socket_block(sck->fd);
   
    /* create tcp server thread runtine */
    if (pthread_create(&sck->ptd, NULL, tcp_server_backup_service, sck))

    {
        printf("sctp server create failed.\n");
        return -1;
    }

    return 0;
}

/**
 * @brief sctp_client_connect 
 *
 * @param sck  [in] socket impl
 * @param ip   [in] server ip
 * @param port [in] server listen port
 *
 * @return 0, if succ; -1, if failed
 */
int sctp_client_connect(struct socket_impl *sck, const char *ip, unsigned short port)
{
    struct sctp_initmsg initmsg = {0};
    struct sctp_event_subscribe event = {0};
    if (sck == NULL || sck->fd > 0) return -1;

    /* init impl */
    memset(sck, 0, sizeof(*sck));
    
    /* create, bind and listen socket */
    if ((sck->fd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0)
    {
        perror("sctp socket failed");
    }

    /* socket bind */
    inet_addr_init(&sck->addr.in_addr, ip, port);

    /* init msg */
    initmsg.sinit_num_ostreams = 5;
    initmsg.sinit_max_instreams = 5;
    initmsg.sinit_max_attempts = 4;
    if (setsockopt(sck->fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg)) < 0)
        perror("sctp setsockopt initmsg failed");

    /* sctp event */
    event.sctp_data_io_event = 1;
    if (setsockopt(sck->fd, IPPROTO_SCTP, SCTP_EVENTS, &event, sizeof(event)) < 0)
        perror("sctp setsockopt event failed");

    /* set socket block */
    make_socket_block(sck->fd);
   
    /* connect server */
    if (!socket_connect(sck->fd, (struct sockaddr *)&sck->addr.addr))
    {
        socket_event_process(sck->fd, sck->evl.on_connect);
    }
    else
    {
        printf("connect failed\n");
        return -1;
    }

    /* create tcp server thread runtine */
    if (pthread_create(&sck->ptd, NULL, tcp_client_backup_service, sck))

    {
        printf("sctp server create failed.\n");
        return -1;
    }

    return 0;
}

/*************************************************************
********  Function Declaration Of Interface Monitor  *********
**************************************************************/
/**
 * @brief linkstatd -- monitor interface status
 *
 * @param link_up   [in] callback when link up
 * @param link_down [in] callback when link down
 *
 * @return 0, if succ; -1, if failed
 */
int linkstatd(void (*link_up)(const char *ifname), void (*link_down)(const char *ifname))
{
    int fd = -1;
    char ifname[64] = {0};
    char msg[1024] = {0};
    struct sockaddr_nl sa = {0};
    struct ifinfomsg *ifi = NLMSG_DATA(msg);

    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK;

    if ((fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
        perror("linkstatd socket create");
    
    socket_bind(fd, (struct sockaddr *)&sa);

    while (read(fd, msg, sizeof(msg)))
    {
        if_indextoname(ifi->ifi_index, ifname);
        if (ifi->ifi_flags & IFF_RUNNING)
        {
            if (link_up != NULL) link_up(ifname);
        }
        else 
        {
            if (link_up != NULL) link_down(ifname);
        }
    }

    return 0;
}
