#ifndef __SOCKET_BASE_H__
#define __SOCKET_BASE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/sctp.h>
#include <fcntl.h>
#include <sys/ioctl.h>

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
int socket_create(int domain, int type);

/**
 * @brief init inet sockaddr_in
 *
 * @param addr   [in] sockaddr_in
 * @param ip     [in] ip address
 * @param port   [in] port
 */
void inet_addr_init(struct sockaddr_in *addr, const char *ip, u_short port);

/**
 * @brief init locak sockaddr_un
 *
 * @param addr   [in] sockaddr_un
 * @param path   [in] path
 */
void local_addr_init(struct sockaddr_un *addr, const char *path);

/**
 * @brief bind a socket。
 *
 * @param addr [in]
 * @param fd   [in] fd of server socket。
 *
 * @return 0, if succ; -1, if failed.
 */
int socket_bind(int fd, struct sockaddr *addr);

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
						u_short port);
						
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
						const char *ip, u_short port);
        
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
int startup_local_server(int type, const char *path);

/**
 * @brief start up a local client socket
 *
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param addr   [in] struct sockaddr_un, cannot be NULL
 * @param path   [in] socket file path, cannot be NULL
 *
 * @return socket fd, if succ; exit, if fail
 */
int startup_local_client(int type, struct sockaddr_un *addr, const char *path);

/**
 * @brief Waiting for an incoming socket.
 *
 * @param fd        [in] server socket fd.
 * @param _cli_addr [in] client sockaddr_in
 *
 * @return new socket fd, if succ; -1, if failed.
 */
int socket_accept(int fd);

/**
 * @brief Connect to server.
 *
 * @param fd       [in] client fd
 * @param cli_addr [in] client sockaddr_in
 *
 * @return 
 */
int socket_connect(int fd, void *cli_addr);

/**
 * @brief Connect to server with a timeout.
 *
 * @param fd       [in] client fd
 * @param cli_addr [in] client sockaddr_in
 * @param tm_ms    [in] time out
 *
 * @return 0, if succ; -1, if fail
 */
int socket_time_connect(int fd, void *cli_addr, int tm_ms);

/**
 * @brief close a socket
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if failed.
 */
int socket_close(int fd);


/*************************************************************
*********  Function Declaration Of TCP Socket Send  **********
**************************************************************/
/**
 * @brief send a message
 *
 * @param fd    [in] socket
 * @param buf   [in] message buffer
 * @param size  [in] size of message buffer
 *
 * @return size of message sended, if succ; -1, if failed.
 */
int socket_send(int fd, void *buf, int size);

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
int socket_time_send(int fd, void *buf, int size, int time_ms);


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
int socket_sendto(int fd, void *buf, int size, const char *ip, int port);

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
int socket_addr_sendto(int fd, void *buf, int size, void *addr);



/*************************************************************
*********  Function Declaration Of TCP Socket Recv  **********
**************************************************************/
/**
 * @brief recveive a message
 *
 * @param fd    [in] socket
 * @param buf   [in] message buffer
 * @param size  [in] size of message buffer
 *
 * @return size of message recveived, if succ; -1, if failed.
 */
int socket_recv(int fd, void *buf, int size);

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
int socket_time_recv(int fd, void *buf, int size, int time_ms);



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
int socket_recvfrom(int fd, void *buf, int size, const char *ip, int port);

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
int socket_addr_recvfrom(int fd, void *buf, int size, void *addr);

#endif
