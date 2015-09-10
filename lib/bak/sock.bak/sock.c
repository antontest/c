#include "sock.h"

/**
 * @brief get local machine's ip address.
 * 
 * @param ip[] [out] local ip address.
 *
 * @return 0, if succ; -1, if failed.
 */
int get_local_ip(char ip[])
{
    struct ifaddrs *ifaddr = NULL;
    void *tmp_addr = NULL;
    char tmp_ip[20] = {0};
    char *hname = NULL;

    if (ip == NULL) return -1;

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
    return socket(domain, type, 0);
}

/**
 * @brief init sockaddr_in
 *
 * @param addr [in]
 * @param ip   [in]
 * @param port [in]
 */
void socket_init(struct sockaddr_in *addr, const char *ip, int port)
{
    /**
     * #define   AF_UNIX    1       local   to   host   (pipes,   portals)
     * #define   AF_INET    2       internetwork:   UDP,   TCP,   etc.
     */
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(port);

    return;
}

/**
 * @brief bind a socket。
 *
 * @param fd   [in] fd of server socket。
 * @param port [in] binding port.
 * @param ip   [in] binding ip address.
 * @param addr [in] sockaddr_in of server.
 *
 * @return 0, if succ; -1, if failed.
 */
int socket_bind(int fd, int port, const char *ip, struct sockaddr_in *addr)
{
    if (fd < 0 || addr == NULL) return -1;

    socket_init(addr, ip, port);

    return bind(fd,(struct sockaddr *)addr,sizeof(struct sockaddr_in));
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
    if (fd < 0) return -1;
    return listen(fd, backlog);
}

/**
 * @brief Waiting for an incoming socket.
 *
 * @param fd        [in] server socket fd.
 * @param _cli_addr [in] client sockaddr_in
 *
 * @return new socket fd, if succ; -1, if failed.
 */
int socket_accept(int fd, struct sockaddr_in *cli_addr)
{
    socklen_t len = sizeof(struct sockaddr);
    
    if (fd < 0) return -1;
    return accept(fd, (struct sockaddr *)cli_addr, &len); 
}

/**
 * @brief Connect to server.
 *
 * @param fd       [in] client fd
 * @param cli_addr [in] client sockaddr_in
 *
 * @return 
 */
int socket_connect(int fd, struct sockaddr_in *cli_addr)
{
    if (fd < 0) return -1;
    return connect(fd, (struct sockaddr *)cli_addr, sizeof(struct sockaddr));
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

/**
 * @brief sock_runtine
 *
 * @param args
 *
 * @return 
 */
void* sock_runtine(void *args)
{
    struct sock *sck = (struct sock *)args;
    struct sockaddr_in cliaddr;
    fd_set set;
    fd_set allset;
    int clifd = -1;
    int serfd = sck->fd;
    int maxfd = sck->fd;
    int fd = -1;
    int maxi = -1;
    int i = 0;
    int nready = 0;
    socklen_t len = -1;
    char recvbuf[128] = {0};

    if (args == NULL) return NULL;

    for (i = 0; i < MAX_CLIENT_NUM; i++)
        sck->cli_fd[i] = -1;

    FD_ZERO(&allset);
    FD_SET(serfd, &allset);
    while (1)
    {
        set = allset;
        nready = select(maxfd + 1, &set, NULL, NULL, NULL);

        if (FD_ISSET(serfd, &set))
        {
            len = sizeof(struct sockaddr_in);
            clifd = accept(serfd, (struct sockaddr *)&cliaddr, &len);

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
                
            FD_SET(clifd, &allset);
            if (clifd > maxfd) maxfd = clifd;
            if (i > maxi) maxi = i;

            if (--nready <= 0) continue;
        }

        for (i = 0; i <= maxi; i++)
        {
            if ((fd = sck->cli_fd[i]) < 0) continue;

            if (FD_ISSET(fd, &set))
            {
                if (read(fd, recvbuf, 128) == 0)
                {
                    close(fd);
                    FD_CLR(fd, &allset);
                    sck->cli_fd[i] = -1;
                }
                else
                {
                    printf("Recv: %s\n", recvbuf);
                }

                if (--nready <= 0) break;
            }
        }
    }

    return NULL;
}

/**
 * @brief create a tcp server
 *
 * @param sck  [in] tcp sock
 * @param port [in] listen port
 *
 * @return 
 */
int tcp_server_create(struct sock *sck, int port)
{
    char ip[20] = {0};
    if (sck->active) return -1;

    get_local_ip(ip);
    sck->fd = socket_create(AF_INET, SOCK_STREAM);
    socket_bind(sck->fd, port, ip, &sck->addr);
    socket_listen(sck->fd, 5);
   
    if (pthread_create(&sck->pthread, NULL, sock_runtine, sck))
    {
        printf("tcp server create failed.\n");
        return -1;
    }

    return 0;
}

/**
 * @brief connect to a tcp server
 *
 * @param sck  [in] client tcp sock
 * @param ip   [in] tcp server ip address
 * @param port [in] tcp server port
 *
 * @return 
 */
int tcp_client_connect(struct sock *sck, const char *ip, int port)
{
    if (sck->active) return -1;

    sck->fd = socket_create(AF_INET, SOCK_STREAM);
    socket_init(&sck->addr, ip, port);

    return socket_connect(sck->fd, &sck->addr);   
}

/**
 * @brief send a message
 *
 * @param sck  [in] socket
 * @param buf  [in] message buffer
 * @param size [in] size of message buffer
 *
 * @return size of message sended, if succ; -1, if failed.
 */
int socket_send(struct sock *sck, void *buf, int size)
{
    return send(sck->fd, buf, size, 0);
}

/**
 * @brief recveive a message
 *
 * @param sck  [in] socket
 * @param buf  [in] message buffer
 * @param size [in] size of message buffer
 *
 * @return size of message recveived, if succ; -1, if failed.
 */
int socket_recv(struct sock *sck, void *buf, int size)
{
    return recv(sck->fd, buf, size, 0);
}

int main()
{
    struct sock ser = {0};
    tcp_server_create(&ser, 5001);
    sleep(180);

    return 0;
}
