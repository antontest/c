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
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/un.h>
#include <netinet/tcp.h>


/*************************************************************
*************************  macro  ****************************
**************************************************************/
#define MAX_CLIENT_NUM 10
#define DFT_STRING_SIZE 128
#define SOCKET_DATA_SIZE sizeof(struct socket_data)
#define SOCKET_DATA_HEADER_SIZE ((unsigned int)&(((struct socket_data *)0)->value))
#ifndef MACRO_STR
#define MACRO_STR(x) {x, #x}
#endif

/*************************************************************
************************  typedef  ***************************
**************************************************************/
typedef struct socket socket_t;
typedef void (*event_cb)(int fd, void *arg);
typedef void* (*thread_runtine)(void *arg);


/*************************************************************
*************************  enum  *****************************
**************************************************************/
typedef enum event_type 
{
    SOCKET_ON_ACCEPT  = 1,
    SOCKET_ON_CONNECT = 2, 
    SOCKET_ON_RECV    = 4,
    SOCKET_ON_SEND    = 8,
    SOCKET_ON_CLOSE   = 16
} event_type_t;

typedef enum socket_data_type
{
    INT    = 0,
    STRING    ,
    CMD       
} socket_data_type;


/*************************************************************
*************************  struct  ***************************
**************************************************************/
/**
 * @brief callback of socket event
 *
 * 1. evt_cb    -- function poiner of callback
 * 2. arg       -- parameter of function
 *
 */
typedef struct callback
{
    /**
     * function poiner of callback 
     */
    event_cb evt_cb;   
    
    /**
     * parameter of function 
     */
    void *arg;        
} callback;

/**
 * @brief event of socket
 *
 * 1. evt       -- type of socket event
 * 2. callback  -- callback of socket event
 *
 */
typedef struct event_loop
{
    /**
     * type of socket event
     */
    event_type_t evt;
    
    /**
     * callback of socket event
     */    
     
    /**
     * happen when accept socket
     */        
    callback on_accept;
    
    /**
     * happen when connected server
     */   
    callback on_connect;
    
    /**
     * happen when socket sending data
     */   
    callback on_send;
    
    /**
     * happen when socket recving data
     */   
    callback on_recv;
    
    /**
     * happen when socket breaking off
     */   
    callback on_close;
} event_loop_t;

/**
 * @brief socket data
 *
 * 1. type      -- type of socket data
 * 2. size      -- size of socket data
 * 3. value     -- socket data
 *
 */
typedef struct socket_data
{
    /**
     * type of socket data
     */   
    socket_data_type type;
    
    /**
     * size of socket data
     */      
    unsigned long size;
    
    /**
     * socket data
     */  
    union {
        int i;
        char s[0];
        void *p;
    } value;
} socket_data_t;

/**
 * @brief sock package
 *
 * 1. fd        -- socket fd
 * 2. cli_fd    -- client socket fd
 * 3. pthread   -- pthread id
 * 4. evl       -- event of socket
 * 5. data      -- socket data
 * 6. addr      -- server socket address struct 
 *
 */
struct socket
{
    /**
     * socket handle
     */  
    int fd;
    
    /**
     * client socket handle
     */  
    int cli_fd[MAX_CLIENT_NUM];
    
    /**
     * thread id
     */   
    pthread_t ptd;
    
    /**
     * event of socket
     */  
    event_loop_t evl;
    
    /**
     * data of socket
     */ 
    socket_data_t data;
    
    /**
     * address of socket
     */ 
    struct sockaddr_in addr;
};

/**
 * @brief name of socket type
 *
 * 1. type_macro   -- system macro of socket type
 * 2. type_name    -- string of socket type
 *
 */
static struct socket_type {
    /**
     * system macro of socket type
     */ 
    int type_macro;

    /**
     * string of socket type
     */ 
    char *type_name;
} socket_type[] = {
    MACRO_STR(SOCK_STREAM)      ,   /* Sequenced, reliable, connection-based  byte streams.  */ 
    MACRO_STR(SOCK_DGRAM)       ,   /* Connectionless, unreliable datagrams  of fixed maximum length.  */
    MACRO_STR(SOCK_RAW)         ,   /* Raw protocol interface.  */
    MACRO_STR(SOCK_RDM)         ,   /* Reliably-delivered messages.  */
    MACRO_STR(SOCK_SEQPACKET)   ,   /* Sequenced, reliable, connection-based,  datagrams of fixed maximum length.  */
    MACRO_STR(SOCK_PACKET)      ,   /* Linux specific way of getting packets  at the dev level.  For writing rarp and  other similar things on the user level. */
    {-1, NULL}
};

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
 * @brief init sockaddr_in
 *
 * @param addr   [in]
 * @param domain [in] This value can be AF_INET,AF_UNIX,AF_LOCAL,
 *                    PF_INET, PF_UINX and PF_LOCAL.
 * @param port   [in] port
 * @param ip     [in] ip address
 */
void socket_addr_init(void *addr, int domain, u_short port, const char *ip);

/**
 * @brief bind a socket。
 *
 * @param addr [in]
 * @param fd   [in] fd of server socket。
 *
 * @return 0, if succ; -1, if failed.
 */
int socket_bind(int fd, void *addr);

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
        const char *ip, int is_ser);

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
int socket_data_send(int fd, socket_data_type type, void *buf, int size);


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
                void *buf, int size, const char *ip, int port);

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
                void *buf, int size, void *addr);


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
int socket_data_recv(int fd, socket_data_type *type, \
                void *buf, int size);

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
                void *buf, int size, int time_ms);


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
int udp_socket_data_recv(int fd, socket_data_type *type, \
                void *buf, int size);

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
            void *buf, int size, const char *ip, int port);

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
            void *buf, int size, void *addr);


/*************************************************************
*********  Function Declaration Of Socket Event  *************
**************************************************************/
/**
 * @brief socket event init 
 *
 * @param evl [in] event loop
 */
void socket_event_init(event_loop_t *evl);

/**
 * @brief add socket event
 *
 * @param evl  [in] event loop
 * @param evt  [in] event type
 * @param cb   [in] event callback
 * @param arg  [in] parameter of callback
 */
void socket_event_add(event_loop_t *evl, event_type_t evt, event_cb cb, void *arg);

/**
 * @brief delete a socket event
 *
 * @param evl [in] event loop
 * @param evt [in] event type
 */
void socket_event_delete(event_loop_t *evl, event_type_t evt);

/**
 * @brief clear all socket events
 *
 * @param evl [in] event loop
 */
void socket_event_clearall(event_loop_t *evl);

/**
 * @brief process socket event
 *
 * @param fd [in] socket fd
 * @param cb [in] event callback
 */
void socket_event_process(int fd, callback cb);


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
int get_local_ip(char ip[]);

/**
 * @brief get bytes which can be readed in the recvive buffer.
 *
 * @param fd [in] socket fd
 *
 * @return data bytes which can be readed, if succ; -1, if failed.
 */
int get_can_read_bytes(int fd);

/**
 * @brief set socket unblock 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1 , if fail
 */
int make_socket_nonblock(int fd);

/**
 * @brief set socket block 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1 , if fail
 */
int make_socket_block(int fd);

/**
 * @brief make listen socket reuseable 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if fail
 */
int make_listen_socket_reuseable(int fd);

/**
 * @brief make socket keep alive 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if fail
 */
int make_socket_keep_alive(int fd);

/**
 * @brief make socket child can't exec 
 *
 * @param fd [in] socket fd
 * 
 * @return 0, if succ; -1, if fail
 */
int make_socket_closenexec(int fd);

/**
 * @brief set size of socket recv buffer 
 *
 * @param fd        [in] socket fd
 * @param buf_size  [in] buffer size
 *
 * @return 0, if succ; -1, if failed.
 */
int set_socket_recv_buf(int fd, int buf_size);

/**
 * @brief set size of socket send buffer 
 *
 * @param fd        [in] socket fd
 * @param buf_size  [in] buffer size
 *
 * @return 0, if succ; -1, if failed.
 */
int set_socket_send_buf(int fd, int buf_size);

/**
 * @brief get size of socket recv buffer 
 *
 * @param fd        [in] socket fd
 *
 * @return recv buffer size, if succ; -1, if failed.
 */
int get_socket_recv_buf(int fd);

/**
 * @brief get size of socket send buffer 
 *
 * @param fd        [in] socket fd
 *
 * @return send buffer size, if succ; -1, if failed.
 */
int get_socket_send_buf(int fd);

/**
 * @brief set socket close action 
 *
 * @param fd    [in] socket fd
 * @param is_on [in] swith of close action
 * @param tm_s  [in] time
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_close_action(int fd, int is_on, int tm_s);

/**
 * @brief set socket broadcast 
 *
 * @param fd [in] socket fd
 * @param on [in] switch
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_broadcast(int fd, int on);

/**
 * @brief set socket multicast loop 
 *
 * @param fd [in] socket fd
 * @param on [in] switch
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_multicast_loop(int fd, int on);

/**
 * @brief set socket multicast ttl 
 *
 * @param fd  [in] socket fd
 * @param ttl [in] ttl
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_multicast_ttl(int fd, int ttl);

/**
 * @brief add socket to multicast member ship
 *
 * @param fd  [in] socket fd
 * @param mrq [in] struct of multicast memver ship
 *
 * @return  0, if succ; -1, if failed.
 */
int add_socket_to_membership(int fd, struct ip_mreq *mrq);

/**
 * @brief drop socket from multicast member ship
 *
 * @param fd  [in] socket fd
 * @param mrq [in] struct of multicast memver ship
 *
 * @return  0, if succ; -1, if failed.
 */
int drop_socket_from_membership(int fd, struct ip_mreq *mrq);

/**
 * @brief get socket timeout of sending 
 *
 * @param fd  [in] socket fd
 *
 * @return send timeout, if succ; -1, if failed
 */
int get_socket_send_timeout(int fd);

/**
 * @brief get socket timeout of recving 
 *
 * @param fd  [in] socket fd
 *
 * @return recv timeout, if succ; -1, if failed
 */
int get_socket_recv_timeout(int fd);

/**
 * @brief set socket timeout of sending 
 *
 * @param fd     [in] socket fd
 * @param tm_ms  [in] timeout
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_send_timeout(int fd, int tm_ms);

/**
 * @brief set socket timeout of recving 
 *
 * @param fd     [in] socket fd
 * @param tm_ms  [in] timeout
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_recv_timeout(int fd, int tm_ms);

/**
 * @brief get socket type 
 *
 * @param fd  [in] socket fd
 *
 * @return socket type, if succ; -1, if failed
 */
int get_socket_type(int fd);

/**
 * @brief get name of socket type
 *
 * @param fd [in] socket fd
 *
 * @return socket type's name, if succ; NULL, if failed.
 */
char* get_socket_type_str(int fd);

/**
 * @brief detect little endian or big endian
 *
 * @return 1, if big endian; 0, if little endian; -1, if unkown.
 */
int is_big_endian();




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
int server_create(struct socket *sck, int domain, int type, \
            u_short port, const char *ip);

/**
 * @brief stop server
 *
 * @param sck [in] sock
 */
void server_stop(struct socket *sck);

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
int client_connect(struct socket *sck, int domain, int type, \
            const char *ip, u_short port);

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
int client_time_connect(struct socket *sck, int domain, int type, \
            const char *ip, u_short port, int tm_ms);



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
        const char *cast_info);

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
        char **cast_info, int size);


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
        const char *cast_info);

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
        char **cast_info, int size);

#endif
