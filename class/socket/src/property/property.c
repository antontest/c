#include "property.h"
#include <sys/time.h>
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
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/un.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <netinet/tcp.h>
#include <netinet/sctp.h>
#include <linux/rtnetlink.h>
#include <netpacket/packet.h>
#include <stdarg.h>
#include <header.h>


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
 * @param family [in] AF_INET or AF_INET6.
 * @param ifname [in] interface name.
 *
 * @return ip address string, if succ; NULL, if failed.
 */
static char local_ip[NI_MAXHOST] = {0};
char *get_local_ip(int family, const char *ifname, int only_first)
{
    const  char *ifname_lo = "lo";
    struct ifaddrs *ifaddr, *ifa;
    char   ip_buf[48] = {0};

    if (getifaddrs(&ifaddr) == -1) return NULL;

    memset(local_ip, 0, sizeof(local_ip));
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        if (family > 0 && family != ifa->ifa_addr->sa_family) continue;
        if (!strcasecmp(ifa->ifa_name, ifname_lo)) continue;
        if (ifname != NULL && strcasecmp(ifa->ifa_name, ifname)) continue;

        switch (ifa->ifa_addr->sa_family) {
            case AF_INET:
                getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), ip_buf, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                strcat(local_ip, ip_buf);
                strcat(local_ip, " ");
                break;
            case AF_INET6:
                getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), ip_buf, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                strcat(local_ip, ip_buf);
                strcat(local_ip, " ");
                break;
            default:
                break;
        }
        
        if (strlen(local_ip) > 0 && only_first > 0) break;
    }
    freeifaddrs(ifaddr);
    
    return local_ip;
}

/**
 * @brief get hardware address by interface name
 *
 * @param ifname [in]  interface name
 * @param mac    [out] hardware addree
 *
 * @return 0, if succ; -1, if failed
 */
static unsigned char local_mac[6] = {0};
unsigned char *get_mac_addr(char *ifname)
{
    int    fd = -1;
    struct ifreq ifreq;
    char   *ifname_ptr = NULL;
    char   *if_save = NULL;
    char   *if_ptr = NULL;

    if (!ifname) ifname = get_ifname(0);
    if (!ifname) return NULL;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return NULL;

    for (ifname_ptr = ifname; ifname_ptr != NULL; ifname_ptr = NULL) {
        if_save = strtok_r(ifname_ptr, " ", &if_ptr);
        if (!if_save) break;
        memset(&ifreq, 0, sizeof(ifreq));
        strcpy(ifreq.ifr_name, if_save);

        if (ioctl(fd, SIOCGIFHWADDR, &ifreq) < 0)
            break;

        memcpy((void *)local_mac, (void *)ifreq.ifr_hwaddr.sa_data, sizeof(local_mac));
    }

    return local_mac;
}

/**
 a* @brief get subnet ip address 
 *
 * @param ip   [in] ip
 * @param mask [in] mask
 * @usage: get_subnet_addr("172.21.34.25", "255.255.0.0")
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
    strcpy(local_ip, inet_ntoa(subnet));

    return local_ip;
}

/**
 * @brief convert mask to bits of mask
 *
 * @param mask [in] mask address
 * @usage: mask_to_bits("255.255.255.0")
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
        return -1;

    if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) < 0)
        return -1;

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
        return -1;

    if (fcntl(fd, F_SETFL, flag & ~O_NONBLOCK) < 0)
        return -1;

    return 0;
}

/**
 * @brief make listen socket reuseable 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if fail
 */
int make_listen_socket_reuseable(int fd, int on)
{
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
}

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
        return -1;

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
        return -1;

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
    int keep_idle = 60;     /* if connection no data exchanges in 60 seconds, then detect */
    int keep_interval = 5;  /* when detecting, the time interval is 5 seconds */
    int keep_count = 3;     /* The number of attempts to detect. \
                               If the first probe packet is received, then the 2 time no longer. */

    /**
     * open keepalive mechanism
     */
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, \
                &keep_alive, sizeof(keep_alive)) == -1)
        return -1;

    /* Default settings are more or less garbage, with the keepalive time 
     * set to 7200 by default on Linux. Modify settings to make the feature 
     * actually useful. */  
  
    /* Send first probe after interval. */  
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, \
                &keep_idle, sizeof(keep_idle)) == -1)
        return -1;

    /* Send next probes after the specified interval. Note that we set the 
     * delay as interval / 3, as we send three probes before detecting 
     * an error (see the next setsockopt call). */  
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, \
                &keep_interval, sizeof(keep_interval)) == -1)
        return -1;

    /* Consider the socket in error state after three we send three ACK 
     * probes without getting a reply. */  
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, \
                &keep_count, sizeof(keep_count)) == -1)
        return -1;

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
    return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));
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
    return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));
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

    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buf_size, (socklen_t *)&len) < 0)
        return -1;
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

    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buf_size, (socklen_t *)&len) < 0)
        return -1;
    return buf_size;
}

/**
 * @brief set socket close action 
 *
 * @param fd     [in] socket fd
 * @param switch [in] swith of close action
 * @param tm_s   [in] time
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_close_action(int fd, int on, int tm_s)
{
    struct linger so_linger = {0};

    so_linger.l_onoff = on;
    so_linger.l_linger = tm_s;

    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger)))
        return -1;
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
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)))
        return -1;
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
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &on, sizeof(on)))
        return -1;
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
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)))
        return -1;
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
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, mrq, sizeof(struct ip_mreq)))
        return -1;
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
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, mrq, sizeof(struct ip_mreq)))
        return -1;
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
    int len = sizeof(tv);

    if (getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, (socklen_t *)&len))
        return -1;
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

    if (getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, (socklen_t *)&len))
        return -1;
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
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, len))
        return -1;
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
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, len))
        return -1;
    return 0;
}

/**
 * @brief get socket protocol 
 *
 * @param fd  [in] socket fd
 *
 * @return protocol type, if succ; -1, if failed
 */
int get_socket_protocol(int fd)
{
    int type = -1;
    int len = sizeof(type);

    if (getsockopt(fd, SOL_SOCKET, SO_PROTOCOL, &type, (socklen_t *)&len))
        return -1;
    return type;
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

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, (socklen_t *)&len))
        return -1;
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
    struct socket_type sock_type[] = {
        MACRO_STR(SOCK_STREAM)      ,   /* Sequenced, reliable, connection-based  byte streams.  */ 
        MACRO_STR(SOCK_DGRAM)       ,   /* Connectionless, unreliable datagrams  of fixed maximum length.  */
        MACRO_STR(SOCK_RAW)         ,   /* Raw protocol interface.  */
        MACRO_STR(SOCK_RDM)         ,   /* Reliably-delivered messages.  */
        MACRO_STR(SOCK_SEQPACKET)   ,   /* Sequenced, reliable, connection-based,  datagrams of fixed maximum length.  */
        MACRO_STR(SOCK_PACKET)      ,   /* Linux specific way of getting packets  at the dev level.  For writing rarp and  other similar things on the user level. */
        {-1, NULL}						/* End */
    };

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, \
                &type, (socklen_t *)&len))
    {
        perror("getsockopt so_sndtimeo");
        return NULL;
    }
    
    while (sock_type[i].type_name != NULL)
    {
        if (sock_type[i].type_macro == type)
            break;
        i++;
    }

    return sock_type[i].type_name;
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
        return -1;

    if (on) req.ifr_flags |= IFF_PROMISC;
    else req.ifr_flags &= ~IFF_PROMISC;

    if (ioctl(fd, SIOCSIFFLAGS, &req) < 0)
        return -1;
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
        return -1;
    return 0;
}

/**
 * @brief get interface name 
 *
 * @param ifname [out] interface name
 *
 * @return 0, if succ; -1, if failed.
 */
static char local_ifname[128] = {0};
char *get_ifname(int only_first)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[2048];

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) return NULL;

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) 
        return NULL;

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    
    memset(local_ifname, 0, sizeof(local_ifname));
    for (; it != end; ++it) 
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) 
        {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) 
            { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) 
                {
		            strcat(local_ifname, ifr.ifr_name);
                    strcat(local_ifname, " ");
                    if (only_first > 0) break;
                }
            }
        }
        else
        {
            printf("get mac info error\n");
            return NULL;
        }
    }

    return local_ifname;
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
            return 1;
        else if (un.c[0] == 0x02 && un.c[1] == 0x01)
            return 0;
        else 
            return -1;
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

/**
 * @brief get_eth_rate
 *
 * @param ifname [in] interface name
 *
 * @return interface rate, if succ; -1, if failed
 */
int get_eth_speed(const char *ifname)
{
    int fd = -1;
    struct ifreq ifr;
    struct ethtool_cmd ep = {0};

    if (ifname == NULL) return -1;

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifname);

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return -1;

    ep.cmd = ETHTOOL_GSET;
    ifr.ifr_data = (caddr_t)&ep;
    if (ioctl(fd, SIOCETHTOOL, &ifr) < 0)
        return -1;

    return ep.speed;
}

/**
 * @brief get_socket_error 
 *
 * @param fd [in] socket
 *
 * @return socket error, if succ; -1, if failed;
 */
int get_socket_error(int fd)
{
    int error = 0;
    socklen_t len = sizeof(error);

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len))
        return -1;
    return error;
}

#define BUFSIZE 8192
struct route_info
{
    u_int dst_addr;
    u_int src_addr;
    u_int gateway;
    char ifname[IF_NAMESIZE];
};
 
static int read_nlsock(int sockfd, char *bufptr, int seqnum, int pid)
{
    struct nlmsghdr *nlhdr;
    int readlen = 0, msglen = 0;
    do{
        //Receive the kernel's respons
        if((readlen = recv(sockfd, bufptr, BUFSIZE - msglen, 0)) < 0)
        {
            perror("SOCK READ: ");
            return -1;
        }
        
        nlhdr = (struct nlmsghdr *)bufptr;
        //Check for header
        if((NLMSG_OK(nlhdr, readlen) == 0) || (nlhdr->nlmsg_type == NLMSG_ERROR))
        {
            perror("Error in recieved packet");
            return -1;
        }
        
        if(nlhdr->nlmsg_type == NLMSG_DONE)
        {
            break;
        }
        else
        {
            bufptr += readlen;
            msglen += readlen;
        }
        
        if((nlhdr->nlmsg_flags & NLM_F_MULTI) == 0)
        {
            break;
        }
    } while((nlhdr->nlmsg_seq != seqnum) || (nlhdr->nlmsg_pid != pid));
    return msglen;
}
 
//Analysis of the returned routing information
static void parse_routes(struct nlmsghdr *nlhdr, struct route_info *rt_info,char *gateway, char *ifname)
{
    struct rtmsg *rt_msg;
    struct rtattr *rt_attr;
    int rt_len;
    struct in_addr dst;
    struct in_addr gate;
 
    rt_msg = (struct rtmsg *)NLMSG_DATA(nlhdr);
    // If the route is not for AF_INET or does not belong to main routing table
    //then return.
    if((rt_msg->rtm_family != AF_INET) || (rt_msg->rtm_table != RT_TABLE_MAIN))
        return;
 
    rt_attr = (struct rtattr *)RTM_RTA(rt_msg);
    rt_len = RTM_PAYLOAD(nlhdr);
    for(; RTA_OK(rt_attr,rt_len); rt_attr = RTA_NEXT(rt_attr,rt_len)){
        switch(rt_attr->rta_type) {
        case RTA_OIF:
            if_indextoname(*(int *)RTA_DATA(rt_attr), rt_info->ifname);
            break;
        case RTA_GATEWAY:
            rt_info->gateway = *(u_int *)RTA_DATA(rt_attr);
            break;
        case RTA_PREFSRC:
            rt_info->src_addr = *(u_int *)RTA_DATA(rt_attr);
            break;
        case RTA_DST:
            rt_info->dst_addr = *(u_int *)RTA_DATA(rt_attr);
            break;
        }
    }

    dst.s_addr = rt_info->dst_addr;
    if (strstr((char *)inet_ntoa(dst), "0.0.0.0"))
    {
        if (ifname != NULL && !strlen(ifname))
            sprintf(ifname, "%s", rt_info->ifname);
        //printf("oif:%s",rt_info->ifname);
        
        gate.s_addr = rt_info->gateway;
        if (gateway != NULL && !strcasecmp(ifname, rt_info->ifname))
            sprintf(gateway, "%s", (char *)inet_ntoa(gate));

        //printf("%s\n",gateway);
        //gate.s_addr = rt_info->src_addr;
        //printf("src:%s\n",(char *)inet_ntoa(gate));
        //gate.s_addr = rt_info->dst_addr;
        //printf("dst:%s\n",(char *)inet_ntoa(gate));
    }

    return;
}
 
/**
 * @brief get_gateway -- get ipaddress of gateway
 *
 * @param gateway [out] ip address of gateway
 * @param ifname  [out] interface name
 *
 * @return 0, if succ; -1; if failed
 */
static char local_gateway[128] = {0};
char *get_gateway()
{
    struct nlmsghdr *nlmsg;
    //struct rtmsg *rt_msg;
    struct route_info *rt_info;
    char msg_buf[BUFSIZE];
    int sock, len, msg_seq = 0;
    char ifname_buf[10] = {0};
    char gateway_buf[48] = {0};
 
    if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
        return NULL;
    memset(msg_buf, 0, BUFSIZE);
 
    nlmsg = (struct nlmsghdr *)msg_buf;
    //rt_msg = (struct rtmsg *)NLMSG_DATA(nlmsg);
 
    nlmsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.
    nlmsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .
 
    nlmsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
    nlmsg->nlmsg_seq = msg_seq++; // Sequence of the message packet.
    nlmsg->nlmsg_pid = getpid(); // PID of process sending the request.
 
    if(send(sock, nlmsg, nlmsg->nlmsg_len, 0) < 0)
        return NULL;
 
    if((len = read_nlsock(sock, msg_buf, msg_seq, getpid())) < 0) 
        return NULL;

    rt_info = (struct route_info *)malloc(sizeof(struct route_info));
    memset(local_gateway, 0, sizeof(local_gateway));
    memset(local_ifname, 0, sizeof(local_ifname));
    for(; NLMSG_OK(nlmsg, len); nlmsg = NLMSG_NEXT(nlmsg, len)){
        memset(rt_info, 0, sizeof(struct route_info));
        parse_routes(nlmsg, rt_info, gateway_buf, ifname_buf);
        if (!strstr(local_gateway, gateway_buf)) {
            strcat(local_gateway, gateway_buf);
            strcat(local_gateway, " ");
        }
        /*
        if (!strstr(local_ifname, ifname_buf)) {
            strcat(local_ifname, ifname_buf);
            strcat(local_ifname, " ");
        }
        */
    }

    free(rt_info);
    close(sock);
   
    return local_gateway;
}

/**
 * @brief xioctl 
 *
 * @param fd      [in] socket fd
 * @param request [in] ioctl command
 * @param argp    [out] value or parameters
 * @param fmt     [in] error info
 * @param ...
 *
 * @return 0, if succ; -1, if failed 
 */
int xioctl(int fd, unsigned int request, void *argp, const char *fmt, ...)
{
    int ret;
    va_list p;
    char buf[256] = {0};

    ret = ioctl(fd, request, argp);
    if (ret < 0)
    {
        va_start(p, fmt);
        vsnprintf(buf, sizeof(buf), fmt, p);
        va_end(p);
        exit(1);
    }

    return ret;
}

/**
 * @brief get_interface_state 
 *
 * @param fd      [in] socket fd
 * @param ifname  [in] interface name
 *
 * @return 1, if up; 0, if down
 */
int get_interface_state(int fd, const char *ifname)
{
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifname);
    xioctl(fd, SIOCGIFFLAGS, (char *)&ifr, NULL);
    return ifr.ifr_flags & IFF_UP ? 1 : 0;
}

/**
 * @brief get_net_mac 
 *
 * @param ip     [in]  target ip address
 * @param mac[6] [out] target mac address
 *
 * @return 0, if uscc; -1, if failed
 */
int get_net_mac(char *dstip, unsigned char mac[6], int timeout)
{
    int fd, len;
    struct sockaddr addr;
    struct frame_arp snd_buf, recv_buf;
    char *ifname = NULL;
    char *if_save = NULL;
    unsigned char *src_mac = NULL;
    unsigned char src_ip[6];
    unsigned char dst_ip[6];
    char src_ip_buf[20];
    struct timeval tv = {0};
    fd_set set;
    struct timeval start = {0}, end = {0};
    int time_use = 0;
    int found_flag = 0;

    if (dstip == NULL || mac == NULL) return -1;

    /**
     * check ifname
     */
    ifname = get_ifname(0);
    if_save = strtok(ifname, " ");
    if (if_save == NULL) return -1;
    
    /**
     * local info
     */
    if_save = get_local_ip(AF_INET, ifname, 1);
    ip2arr(src_ip_buf, src_ip);
    ip2arr(dstip, dst_ip);
    src_mac = get_mac_addr(if_save);
    if (!memcmp(dst_ip, src_ip, 4)) {
        memcpy(mac, src_mac, 6);
        return 1;
    }

    /**
     * init addr
     */
    memset(&addr, 0, sizeof(addr));
    addr.sa_family = PF_PACKET;
    strcpy(addr.sa_data, if_save);

    /**
     * create socket
     */
    if ((fd = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) < 0) {
        perror("socket()");
        exit(1);
    }

    /**
     * bind socket
     */
    if (bind(fd, &addr, sizeof(addr)) < 0) {
        perror("bind()");
        exit(1);
    }

    /**
     * init arp request packet
     */
    memcpy(snd_buf.fh.src_mac, src_mac, 6);
    memcpy(snd_buf.ah.src_mac, src_mac, 6);
    memset(snd_buf.fh.dst_mac, -1, 6);
    memset(snd_buf.ah.dst_mac, 0, 6);
    snd_buf.fh.protocol = htons(ETH_P_ARP);
    snd_buf.ah.ar_hrd = htons(ARPHRD_ETHER);
    snd_buf.ah.ar_pro = htons(ETH_P_IP);
    snd_buf.ah.ar_hln = 6;
    snd_buf.ah.ar_pln = 4;
    snd_buf.ah.ar_op = htons(ARPOP_REQUEST);
    memcpy(snd_buf.ah.dst_ip, dst_ip, 4);
    memcpy(snd_buf.ah.src_ip, src_ip, 4);

    len = sizeof(struct sockaddr);
    sendto(fd, &snd_buf, sizeof(snd_buf), 0, &addr, len);
    FD_ZERO(&set);
    FD_SET(fd, &set);
    gettimeofday(&start, NULL);
    while (1) {
        tv.tv_sec = 0;
        tv.tv_usec = 1000 * 500;
        FD_ZERO(&set);
        FD_SET(fd, &set);

        //sendto(fd, &snd_buf, sizeof(snd_buf), 0, &addr, len);
        select(fd + 1, &set, NULL, NULL, &tv);
        if (FD_ISSET(fd, &set))
        {
            recvfrom(fd, &recv_buf, sizeof(recv_buf), 0, NULL, NULL);
            if (!memcmp(recv_buf.ah.src_ip, dst_ip, 4)) 
            {
                memcpy(mac, recv_buf.ah.src_mac, 6);
                found_flag = 1;
                break;
            }
        }

        gettimeofday(&end, NULL);
        time_use = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        if (timeout > 0 && (time_use / 1000000) >= timeout) 
        {
            break;
        }
    }
    gettimeofday(&end, NULL);
    time_use = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    
    if (!found_flag) return found_flag;
    return time_use;
}

/**
 * @brief ip2arr -- The IP address of the string is converted to an array
 *
 * @param ip     [in]  string of ip address
 * @param arr[4] [out] array of ip address
 */
void ip2arr(const char *ip, unsigned char ip_arr[4])
{
    unsigned char n = 0;
    int i = 0;

    if (ip == NULL) return;

    while (*ip != '\0' && i < 4) {
        while (*ip >= '0' && *ip <= '9') {
            n = n * 10 + (*ip - '0');
            ip++;
        }
        ip_arr[i++] = n;
        n = 0;
        ip++;
    }
    n = 0;
}

/**
 * @brief arr2ip 
 *
 * @param ip_arri [in] array of ip address
 * @param ip     [out] string of ip address
 */
void arr2ip(unsigned char ip_arr[], char ip[])
{

    if (ip_arr == NULL || ip == NULL) return;
    sprintf(ip, "%d.%d.%d.%d", ip_arr[0], ip_arr[1], ip[2], ip_arr[3]);
}

/**
 * @brief arr2mac
 *
 * @param mac_arr [in] array of mac address
 * @param mac[]  [out] string of mac address 
 */
void mac2arr(const char *mac, unsigned char mac_arr[6])
{
    unsigned char n = 0;
    int i = 0;

    if (mac == NULL) return;

    while (i < 6 && *mac != '\0') 
    {
        if (*mac >= '0' && *mac <= '9')
            n += (*mac - '0') * 16;
        else if (*mac >= 'a' && *mac <= 'f') 
            n+= (*mac - 'a' + 10) * 16;
        else if (*mac >= 'A' && *mac <= 'F') 
            n+= (*mac - 'A' + 10) * 16;
        else return;

        mac++;
        if (*mac >= '0' && *mac <= '9')
            n += *mac - '0';
        else if (*mac >= 'a' && *mac <= 'f') 
            n+= *mac - 'a' + 10;
        else if (*mac >= 'A' && *mac <= 'F') 
            n+= *mac - 'A' + 10;
        else return;

        mac_arr[i++] = n;
        mac++;
        if (*mac != ':' || *mac == '\0' || n < 0) 
        {
            strcpy((char *)mac_arr, "\0");
            return;
        }

        n = 0;
        mac++;
    }
}

/**
 * @brief arr2mac
 *
 * @param mac_arr [in] array of mac address
 * @param mac[]  [out] string of mac address 
 */
void arr2mac(const unsigned char *mac_arr, char mac[])
{
    if (mac_arr == NULL || mac == NULL) return;
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", mac_arr[0], mac_arr[1], 
            mac_arr[2], mac_arr[3], mac_arr[4], mac_arr[5]);
}

/**
 * @brief printf_mac 
 *
 * @param mac  [in] mac
 * @param info [in] info
 */
void print_mac(const unsigned char *mac, const char *info)
{
    if (info == NULL)
        printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], 
                mac[1], mac[2], mac[3], mac[4], mac[5]);
    else 
        printf("%s: %02x:%02x:%02x:%02x:%02x:%02x\n", info, mac[0], 
                mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**
 * @brief printf_mac 
 *
 * @param mac  [in] mac
 * @param info [in] info
 */
void print_ipv4(const unsigned char *ip, const char *info)
{
    if (info == NULL)
        printf("ip: %d.%d.%d.%d\n", ip[0], ip[1], 
                ip[2], ip[3]);
    else
        printf("%s: %d.%d.%d.%d\n", info, ip[0], ip[1], 
                ip[2], ip[3]);
}

