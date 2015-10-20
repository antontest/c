#ifndef __SOCKET_APP_H__
#define __SOCKET_APP_H__

#ifndef __SOCKET_BASE_H__
#include "socket_base.h"
#endif

#ifndef __SOCKET_PROPERTY_H__
#include "socket_property.h"
#endif

#ifndef __SOCKET_EVENT_H__
#include "socket_event.h"
#endif

#ifndef __SOCKET_HEADER_H__
#define __SOCKET_HEADER_H__
#include "socket_header.h"
#endif

#include <pthread.h>
#include <netpacket/packet.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/sctp.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <pcap.h>

/*************************************************************
*************************  macro  ****************************
**************************************************************/
#define MAX_CLIENT_NUM 10
#define DFT_STRING_SIZE 128
#define SOCKET_DATA_SIZE sizeof(struct socket_data)
#define SOCKET_DATA_HEADER_SIZE ((unsigned int)&(((struct socket_data *)0)->value))

/*************************************************************
************************  typedef  ***************************
**************************************************************/
typedef void* (*thread_runtine)(void *arg);

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
typedef struct socket_impl
{
    /**
     * socket handle
     */  
    int fd;
    
    /**
     * client socket handle
     */  
    int cli_fd[MAX_CLIENT_NUM];
    int curr_cli_fd;
    
    /**
     * thread id
     */   
    pthread_t ptd;
    
    /**
     * event of socket
     */  
    event_loop_t evl;
    
    /**
     * address of socket
     */ 
    union {
        struct sockaddr_in in_addr;
        struct sockaddr_un un_addr;
        struct sockaddr addr;
    } addr;
    
    /**
     * message of socket
     */ 
    char body[0];
} socket_impl_t;

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
        char *cast_info, int size);


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
        char *cast_info, int size);
        
        
/*************************************************************
********  Function Declaration Of Socket Application  ********
**************************************************************/
/**
 * @brief create a server
 *
 * @param sck      [in] tcp sock
 * @param type     [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param port     [in] listen port
 *
 * @return 0, if succ; -1, if failed.
 */
int inet_server_create(struct socket_impl *sck, int type, \
                       const char *ip,  u_short port);
            
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
int local_server_create(struct socket_impl *sck, int type, const char *path);

/**
 * @brief stop server
 *
 * @param sck [in] sock
 */
void server_stop(struct socket_impl *sck);

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
int inet_client_connect(struct socket_impl *sck, int type, \
                        const char *ip, u_short port);

/**
 * @brief connect to a server
 *
 * @param sck    [in] client tcp sock
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param path   [in] tcp server ip address
 *
 * @return 0, if succ; -1, if fail 
 */
int local_client_connect(struct socket_impl *sck, int type, const char *path);
            
/**
 * @brief connect to a server
 *
 * @param sck   [in] client tcp sock
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param ip    [in] tcp server ip address
 * @param port  [in] tcp server port
 * @param tm_ms [in] connect timeout
 *
 * @return 0, if succ; -1, if fail
 */
int inet_client_time_connect(struct socket_impl *sck, int type, \
                            const char *ip, u_short port, int tm_ms);
            
/**
 * @brief connect to a server
 *
 * @param sck    [in] client tcp sock
 * @param type   [in] can be SOCK_TREAM, SOCK_DGRAM.
 * @param path   [in] tcp server ip address
 * @param tm_ms  [in] connect timeout
 *
 * @return 0, if succ; -1, if fail
 */
int local_client_time_connect(struct socket_impl *sck, int type, const char *path, int tm_ms);

/**
 * @brief ethdump socket init 
 *
 * @return socket fd, if succ; -1, if failed.
 */
int raw_socket_init();

int parse_ether_head(void *data, unsigned char dhost[6], \
        unsigned char shost[6], unsigned short *proto_type);
        
int parse_icmp_head(void *data, unsigned char *type, unsigned char *code);

int parse_ping(void *data);

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
int sctp_server_create(struct socket_impl *sck, const char *ip, unsigned short port);

/**
 * @brief sctp_client_connect 
 *
 * @param sck  [in] socket impl
 * @param ip   [in] server ip
 * @param port [in] server listen port
 *
 * @return 0, if succ; -1, if failed
 */
int sctp_client_connect(struct socket_impl *sck, const char *ip, unsigned short port);



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
int linkstatd(void (*link_up)(const char *ifname), void (*link_down)(const char *ifname));


#endif
