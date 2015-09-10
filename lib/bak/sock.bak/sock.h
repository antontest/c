#ifndef __SOCK_H__
#define __SOCK_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

#define MAX_CLIENT_NUM 10

struct sock
{
    int fd;
    int cli_fd[MAX_CLIENT_NUM];
    int conn_flag;
    int active;
    union {
        int i;
        char *sz;
        void *p;
    };
    pthread_t pthread;
    struct sockaddr_in addr;
};

/**
 * @brief get local machine's ip address.
 * 
 * @param ip[] [out] local ip address.
 *
 * @return 0, if succ; -1, if failed.
 */
int get_local_ip(char ip[]);

/**
 * @brief create a soocket
 *
 * @param domain [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 *
 * @return socket fd, if succ; -1, if failed.
 */
int socket_create(int domain, int type);

/**
 * @brief init sockaddr_in
 *
 * @param addr [in]
 * @param ip   [in]
 * @param port [in]
 */
void socket_init(struct sockaddr_in *addr, const char *ip, int port);

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
int socket_bind(int fd, int port, const char *ip, struct sockaddr_in *addr);

/**
 * @brief  listening for an incoming connect.
 *
 * @param fd      [in] listen socket fd.
 * @param backlog [in] Maximum length of the queue of pending connections.
 *
 * @return 0, if succ; -1, if failed.
 */
int socket_listen(int fd, int backlog);

/**
 * @brief Waiting for an incoming socket.
 *
 * @param fd        [in] server socket fd.
 * @param _cli_addr [in] client sockaddr_in
 *
 * @return new socket fd, if succ; -1, if failed.
 */
int socket_accept(int fd, struct sockaddr_in *cli_addr);

/**
 * @brief Connect to server.
 *
 * @param fd       [in] client fd
 * @param cli_addr [in] client sockaddr_in
 *
 * @return 
 */
int socket_connect(int fd, struct sockaddr_in *cli_addr);

/**
 * @brief close a socket
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if failed.
 */
int socket_close(int fd);

/**
 * @brief sock_runtine
 *
 * @param args
 *
 * @return 
 */
void* sock_runtine(void *args);

/**
 * @brief create a tcp server
 *
 * @param sck  [in] tcp sock
 * @param port [in] listen port
 *
 * @return 
 */
int tcp_server_create(struct sock *sck, int port);

/**
 * @brief connect to a tcp server
 *
 * @param sck  [in] client tcp sock
 * @param ip   [in] tcp server ip address
 * @param port [in] tcp server port
 *
 * @return 
 */
int tcp_client_connect(struct sock *sck, const char *ip, int port);

int socket_send(struct sock *sck, void *buf, int size);
int socket_recv(struct sock *sck, void *buf, int size);
#endif
