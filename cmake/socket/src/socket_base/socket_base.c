#include "socket_base.h"

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

/**
 * @brief set socket unblock 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1 , if fail
 */
static int set_socket_nonblock(int fd)
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
 * @brief set socket timeout of sending 
 *
 * @param fd     [in] socket fd
 * @param tm_ms  [in] timeout
 *
 * @return 0, if succ; -1, if failed
 */
int set_socket_send_timeout(int fd, int tm_ms)
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
int socket_create(int domain, int type, int protocol)
{
    int fd = socket(domain, type, protocol);
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
 * @param ip     [in] ip address, can be NULL
 * @param port   [in] socket port
 *
 * @return socket fd, if succ; exit, if fail
 */
int startup_inet_server(int type, const char *ip, \
						u_short port)
{
    int fd = -1;
    struct sockaddr_in addr = {0};
	
    /* create socket */
    if (type == SOCK_STREAM)
        fd = socket_create(AF_INET, type, IPPROTO_TCP);
    else
        fd = socket_create(AF_INET, type, IPPROTO_UDP);

    /* init sockaddr_in */
    inet_addr_init(&addr, ip, port);
    
    /* bind socket */
    socket_bind(fd, (struct sockaddr *)&addr);
    
    /* build socket listen */
    if (type == SOCK_STREAM) socket_listen(fd, 5);

    return fd;
}

/**
 * @brief start up a internet client socket
 *
 * @param domain [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param ip     [in] ip address, can be NULL. if ip is NULL, connect to local
 * @param port   [in] socket port
 *
 * @return socket fd, if succ; exit, if fail
 */
int startup_inet_client(int type, struct sockaddr_in *addr, \
						const char *ip, u_short port)
{

    /* init sockaddr_in */
    if (addr == NULL)
    {
        fprintf(stderr, "%s", "struct sockaddr_in cannot be NULL\n");
        exit(1);
    }
	if (ip != NULL)
		inet_addr_init(addr, ip, port);
	else inet_addr_init(addr, "127.0.0.1", port);

	/* create socket */
	if (type == SOCK_STREAM)
        return socket_create(AF_INET, type, IPPROTO_TCP);
    else
        return socket_create(AF_INET, type, IPPROTO_UDP);
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
int startup_local_server(int type, const char *path)
{
    int fd = -1;
    struct sockaddr_un addr = {0};
    
    /* init sockaddr_in */
    local_addr_init(&addr, path);
	
    /* create socket */
    fd = socket_create(AF_UNIX, type, IPPROTO_TCP);
	unlink(addr.sun_path);
    
    /* bind and listen socket */
    socket_bind(fd, (struct sockaddr *)&addr);
    if (type == SOCK_STREAM) socket_listen(fd, 5);
	
    return fd;  
}

/**
 * @brief start up a local client socket
 *
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param addr   [in] struct sockaddr_un, cannot be NULL
 * @param path   [in] socket file path, cannot be NULL
 *
 * @return socket fd, if succ; exit, if fail
 */
int startup_local_client(int type, struct sockaddr_un *addr, const char *path)
{
    if (addr == NULL)
	{
		fprintf(stderr, "addr cannot be NULL\n");
		exit(1);
	}
	
    /* init sockaddr_in */
    local_addr_init(addr, path);
	
    /* create socket */
    return socket_create(AF_UNIX, type, IPPROTO_TCP);  
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
    set_socket_nonblock(fd);
    
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
    
    set_socket_send_timeout(fd, time_ms);
    rt = send(fd, buf, size, 0);
    set_socket_send_timeout(fd, 0);

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
        ioctl(fd, FIONREAD, &can_recv_bytes);
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
