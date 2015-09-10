#ifndef __SOCKET_PROPERTY_H__
#define __SOCKET_PROPERTY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/un.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <netinet/tcp.h>
#include <netinet/sctp.h>

/*************************************************************
*************************  macro  ****************************
**************************************************************/
#ifndef MACRO_STR
#define MACRO_STR(x) {x, #x}
#endif

#ifndef SIOCETHTOOL
#define SIOCETHTOOL 0x8946
#endif

#define ETHTOOL_GSET 0x00000001 /* Get settings */
#define ETHTOOL_SSET 0x00000002 /* Set settings */

/* The forced speed, 10Mb, 100Mb, gigabit, 2.5Gb, 10GbE. */  
#define SPEED_10        10  
#define SPEED_100       100  
#define SPEED_1000      1000  
#define SPEED_2500      2500  
#define SPEED_10000     10000  
  
/* This should work for both 32 and 64 bit userland. */  
struct ethtool_cmd {  
    __u32   cmd;  
    __u32   supported;      /* Features this interface supports */  
    __u32   advertising;    /* Features this interface advertises */  
    __u16   speed;          /* The forced speed, 10Mb, 100Mb, gigabit */  
    __u8    duplex;         /* Duplex, half or full */  
    __u8    port;           /* Which connector port */  
    __u8    phy_address;  
    __u8    transceiver;    /* Which transceiver to use */  
    __u8    autoneg;        /* Enable or disable autonegotiation */  
    __u32   maxtxpkt;       /* Tx pkts before generating tx int */  
    __u32   maxrxpkt;       /* Rx pkts before generating rx int */  
    __u32   reserved[4];  
};  

/*************************************************************
*************************  struct  ***************************
**************************************************************/
/**
 * @brief name of socket type
 *
 * 1. type_macro   -- system macro of socket type
 * 2. type_name    -- string of socket type
 *
 */
struct socket_type {
    /**
     * system macro of socket type
     */ 
    int type_macro;

    /**
     * string of socket type
     */ 
    char *type_name;
} ;


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
int get_ip(char ip[]);

/**
 * @brief get ip address by ifname 
 *
 * @param ifname [in]  interface name
 * @param ip     [out] ip address 
 *
 * @return 0, if succ; -1, if failed
 */
int get_ip_by_ifname(const char *ifname, char *ip);

/**
 * @brief get hardware address by interface name
 *
 * @param ifname [in]  interface name
 * @param mac    [out] hardware addree
 *
 * @return 0, if succ; -1, if failed
 */
int get_mac_addr(const char *ifname, char *mac);

/**
 a* @brief get subnet ip address 
 *
 * @param ip   [in] ip
 * @param mask [in] mask
 *
 * @return subnet ip address, if succ;
 */
char * get_subnet_addr(const char *ip, const char *mask);

/**
 * @brief convert mask to bits of mask
 *
 * @param mask [in] mask address
 *
 * @return bits of mask, if succ; -1, if failed
 */
int mask_to_bits(const char *mask);

/**
 * @brief get interface name 
 *
 * @param ifname [out] interface name
 *
 * @return 0, if succ; -1, if failed.
 */
int get_ifname(char *ifname);

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
 * @brief get socket protocol 
 *
 * @param fd  [in] socket fd
 *
 * @return protocol type, if succ; -1, if failed
 */
int get_socket_protocol(int fd);

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
 * @brief make network card hybrid mode
 *
 * @param ifname [in] interface name like eth0,eth2 and so on
 * @param fd     [in] socket fd
 * @param on     [in] hybrid mode swith
 *
 * @return 0, if succ; -1, if failed. 
 */
int make_socket_promisc(const char *ifname, int fd, int on);

/**
 * @brief get interface index 
 *
 * @param fd   [in] socket fd
 * @param req  [out] struct ifreq, return interface name
 *
 * @return 0, if succ; -1, if failed.
 */
int get_interface_index(int fd, struct ifreq *req);

/**
 * @brief detect little endian or big endian
 *
 * @return 1, if big endian; 0, if little endian; -1, if unkown.
 */
int is_big_endian();

/**
 * @brief ip is legal
 *
 * @param ip [in] ip address
 *
 * @return 1, if legal; 0, if illage
 */
int match_ip(const char *ip);

/**
 * @brief get_eth_rate
 *
 * @param ifname [in] interface name
 *
 * @return interface rate, if succ; -1, if failed
 */
int get_eth_speed(const char *ifname);

#endif
