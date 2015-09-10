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
    memset(sck, 0, sizeof(*sck));
    
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
    memset(sck, 0, sizeof(*sck));
    
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
    memset(sck, 0, sizeof(*sck));
    
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
    memset(sck, 0, sizeof(*sck));
    
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