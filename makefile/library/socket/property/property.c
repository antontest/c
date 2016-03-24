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
#include <utils.h>

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

typedef struct private_property_t private_property_t;
struct private_property_t {
    /**
     * @brief public interface 
     */
    property_t public;

    /**
     * @brief socket descriptor
     */
    int fd;
};
#define socket_fd     this->fd
#define socket_buffer this->buffer

METHOD(property_t, get_can_read_bytes_, int, private_property_t *this)
{
    int can_read_bytes = -1;

    ioctl(socket_fd, FIONREAD, &can_read_bytes);
    return can_read_bytes;
}

METHOD(property_t, get_recv_buf_size_, int, private_property_t *this)
{
    int buf_size = 0;
    int len = sizeof(buf_size);

    if (getsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &buf_size, (socklen_t *)&len) < 0)
        return -1;
    return buf_size;
}

METHOD(property_t, get_send_buf_size_, int, private_property_t *this)
{
    int buf_size = 0;
    int len = sizeof(buf_size);

    if (getsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &buf_size, (socklen_t *)&len) < 0)
        return -1;
    return buf_size;
}

METHOD(property_t, get_recv_timeout, int, private_property_t *this)
{
    struct timeval tv = {0};
    int len = sizeof(tv);

    if (getsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, (socklen_t *)&len))
        return -1;
    return (tv.tv_sec * 1000 + tv.tv_usec);
}

METHOD(property_t, get_send_timeout, int, private_property_t *this)
{
    struct timeval tv = {0};
    int len = sizeof(tv);

    if (getsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, (socklen_t *)&len))
        return -1;
    return (tv.tv_sec * 1000 + tv.tv_usec);
}

METHOD(property_t, get_protocol_, int, private_property_t *this)
{
    int type = -1;
    int len = sizeof(type);

    if (getsockopt(socket_fd, SOL_SOCKET, SO_PROTOCOL, &type, (socklen_t *)&len))
        return -1;
    return type;
}

METHOD(property_t, get_family_, int, private_property_t *this)
{
    int type = -1;
    int len = sizeof(type);

    if (getsockopt(socket_fd, SOL_SOCKET, SO_TYPE, &type, (socklen_t *)&len))
        return -1;
    return type;
}

METHOD(property_t, get_error_, int, private_property_t *this)
{
    int error = 0;
    socklen_t len = sizeof(error);

    if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &error, &len))
        return -1;
    return error;
}

METHOD(property_t, get_interface_state_, int, private_property_t *this, const char *ifname)
{
    struct ifreq ifr;

    if (!ifname) return -1;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifname);
    xioctl(socket_fd, SIOCGIFFLAGS, (char *)&ifr, NULL);
    return ifr.ifr_flags & IFF_UP ? 1 : 0;
}


METHOD(property_t, set_recv_buf_size_, int, private_property_t *this, int size)
{
    return setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

METHOD(property_t, set_send_buf_size_, int, private_property_t *this, int size)
{
    return setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

METHOD(property_t, set_recv_timeout_, int, private_property_t *this, int tm_ms)
{
    struct timeval tv = {0};
    int len = sizeof(tv);

    tv.tv_sec = tm_ms / 1000;
    tv.tv_usec = tm_ms % 1000;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, len))
        return -1;
    return 0;
}

METHOD(property_t, set_send_timeout_, int, private_property_t *this, int tm_ms)
{
    struct timeval tv = {0};
    int len = sizeof(tv);

    tv.tv_sec = tm_ms / 1000;
    tv.tv_usec = tm_ms % 1000;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, len))
        return -1;
    return 0;
}


METHOD(property_t, make_nonblock_, int, private_property_t *this)
{
    int flag = 0;
    
    if ((flag = fcntl(socket_fd, F_GETFL, 0)) < 0)
        return -1;

    if (fcntl(socket_fd, F_SETFL, flag | O_NONBLOCK) < 0)
        return -1;

    return 0;
}

METHOD(property_t, make_block_, int, private_property_t *this)
{
    int flag = 0;
    
    if ((flag = fcntl(socket_fd, F_GETFL, 0)) < 0)
        return -1;

    if (fcntl(socket_fd, F_SETFL, flag & ~O_NONBLOCK) < 0)
        return -1;

    return 0;
}

METHOD(property_t, make_reuseable, int, private_property_t *this, int onoff)
{
    return setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&onoff, sizeof(onoff));
}

METHOD(property_t, make_closenexec_, int, private_property_t *this)
{
    int flags = 0;

    if ((flags = fcntl(socket_fd, F_GETFD, NULL)) < 0) 
        return -1;

    if (fcntl(socket_fd, F_SETFD, flags | FD_CLOEXEC) == -1)
        return -1;

    return 0;
}

METHOD(property_t, make_keepalive_, int, private_property_t *this)
{
    int keep_alive = 1;     /* open keep alive attribute */
    int keep_idle = 60;     /* if connection no data exchanges in 60 seconds, then detect */
    int keep_interval = 5;  /* when detecting, the time interval is 5 seconds */
    int keep_count = 3;     /* The number of attempts to detect. \
                               If the first probe packet is received, then the 2 time no longer. */

    /**
     * open keepalive mechanism
     */
    if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, \
                &keep_alive, sizeof(keep_alive)) == -1)
        return -1;

    /* Default settings are more or less garbage, with the keepalive time 
     * set to 7200 by default on Linux. Modify settings to make the feature 
     * actually useful. */  
  
    /* Send first probe after interval. */  
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, \
                &keep_idle, sizeof(keep_idle)) == -1)
        return -1;

    /* Send next probes after the specified interval. Note that we set the 
     * delay as interval / 3, as we send three probes before detecting 
     * an error (see the next setsockopt call). */  
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, \
                &keep_interval, sizeof(keep_interval)) == -1)
        return -1;

    /* Consider the socket in error state after three we send three ACK 
     * probes without getting a reply. */  
    if (setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, \
                &keep_count, sizeof(keep_count)) == -1)
        return -1;

    return 0;
}

METHOD(property_t, make_close_action_, int, private_property_t *this, int onoff, int tm)
{
    struct linger so_linger = {0};

    so_linger.l_onoff  = onoff;
    so_linger.l_linger = tm;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger)))
        return -1;
    return 0;
}

METHOD(property_t, make_broadcast_, int, private_property_t *this, int onoff)
{
    if (setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &onoff, sizeof(onoff)))
        return -1;
    return 0;
}

METHOD(property_t, make_promisc_, int, private_property_t *this, const char *ifname, int onoff)
{
    struct ifreq req;

    strcpy(req.ifr_name, ifname);
    if (ioctl(socket_fd, SIOCGIFFLAGS, &req) < 0)
        return -1;

    if (onoff) req.ifr_flags |= IFF_PROMISC;
    else req.ifr_flags &= ~IFF_PROMISC;

    if (ioctl(socket_fd, SIOCSIFFLAGS, &req) < 0)
        return -1;
    return 0;
}

METHOD(property_t, make_multicast_loop_, int, private_property_t *this, int onoff)
{
    if (setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &onoff, sizeof(onoff)))
        return -1;
    return 0;
}

METHOD(property_t, make_multicast_ttl_, int, private_property_t *this, int ttl)
{
    if (setsockopt(socket_fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)))
        return -1;
    return 0;

}

METHOD(property_t, add_to_membership_, int, private_property_t *this, struct ip_mreq *mrq)
{
    if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, mrq, sizeof(struct ip_mreq)))
        return -1;
    return 0;
}

METHOD(property_t, drop_from_membership_, int, private_property_t *this, struct ip_mreq *mrq)
{
    if (setsockopt(socket_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, mrq, sizeof(struct ip_mreq)))
        return -1;
    return 0;

}

METHOD(property_t, set_fd_, void, private_property_t *this, int fd)
{
    socket_fd = fd;
}

METHOD(property_t, destroy_, void, private_property_t *this)
{
    free(this);
}

property_t *create_property(int fd)
{
    private_property_t *this;

    INIT(this,
        .public = {
            .get_can_read_bytes = _get_can_read_bytes_,
            .get_recv_buf_size  = _get_recv_buf_size_,
            .get_send_buf_size  = _get_send_buf_size_,
            .get_recv_timeout   = _get_recv_timeout,
            .get_send_timeout   = _get_send_timeout,
            .get_protocol       = _get_protocol_,
            .get_family         = _get_family_,
            .get_error          = _get_error_,
            .get_interface_state = _get_interface_state_,

            .set_recv_buf_size  = _set_recv_buf_size_,
            .set_send_buf_size  = _set_send_buf_size_,
            .set_recv_timeout   = _set_recv_timeout_,
            .set_send_timeout   = _set_send_timeout_,

            .make_block      = _make_block_,
            .make_nonblock   = _make_nonblock_,
            .make_reuseable  = _make_reuseable,
            .make_closenexec = _make_closenexec_,
            .make_keepalive  = _make_keepalive_,
            .make_broadcast  = _make_broadcast_,
            .make_close_action = _make_close_action_,
            .make_promisc    = _make_promisc_,
            .make_multicast_loop = _make_multicast_loop_,
            .make_multicast_ttl  = _make_multicast_ttl_,
            .add_to_membership    = _add_to_membership_,
            .drop_from_membership = _drop_from_membership_,

            .set_fd  = _set_fd_,
            .destroy = _destroy_,
        },
        .fd     = fd,
    );

    return &this->public;
}


char *get_local_ip(int family, const char *ifname, char *ip, int size)
{
    const  char *ifname_lo = "lo";
    struct ifaddrs *ifaddr, *ifa;
    char   ip_buf[48] = {0};
    int    used_cnt   = 0;

    if (getifaddrs(&ifaddr) == -1) return NULL;
    if (!ip) return NULL;

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        if (family > 0 && family != ifa->ifa_addr->sa_family) continue;
        if (!strcasecmp(ifa->ifa_name, ifname_lo)) continue;
        if (ifname != NULL && strcasecmp(ifa->ifa_name, ifname)) continue;

        switch (ifa->ifa_addr->sa_family) {
            case AF_INET:
                getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), ip_buf, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                used_cnt += snprintf(ip + used_cnt, size - 1, "%s ", ip_buf);
                break;
            case AF_INET6:
                getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), ip_buf, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                used_cnt += snprintf(ip + used_cnt, size - 1, "%s ", ip_buf);
            default:
                break;
        }
        if (used_cnt >= size) break;
    }
    ip[used_cnt] = '\0';
    freeifaddrs(ifaddr);
    
    return ip;
}

unsigned char *get_mac(const char *ifname, unsigned char *mac, int size)
{
    struct ifreq ifreq;
    int fd = -1;

    if (!mac) return NULL;
    if (!ifname) return NULL;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return NULL;

    memset(&ifreq, 0, sizeof(ifreq));
    strncpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name));
    if (ioctl(fd, SIOCGIFHWADDR, &ifreq) < 0)
        return NULL;

    memcpy((void *)mac, (void *)ifreq.ifr_hwaddr.sa_data, size);
    return mac;
}

char *get_ifname(char *ifname, int size)
{
    struct ifreq ifr;
    struct ifconf ifc;
    int used_cnt  = 0;
    char buf[256];

    if (!ifname || size < 1) return NULL;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) return NULL;

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) 
        return NULL;

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    
    for (; it != end; ++it) 
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) 
        {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) 
            { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) 
                {
		            used_cnt += snprintf(ifname + used_cnt, size - 1, "%s ", ifr.ifr_name);
                }
            }
        }
        else continue;
    }

    return ifname;
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

    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
        return -1;

    ep.cmd = ETHTOOL_GSET;
    ifr.ifr_data = (caddr_t)&ep;
    if (ioctl(fd, SIOCETHTOOL, &ifr) < 0)
        return -1;

    return ep.speed;
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
 * @return 0, if succ; -1; if failed
 */
char *get_gateway(char *gateway, int size)
{
    struct nlmsghdr *nlmsg;
    //struct rtmsg *rt_msg;
    struct route_info *rt_info;
    char msg_buf[BUFSIZE];
    int sock, len, msg_seq = 0;
    char ifname_buf[10]  = {0};
    char gateway_buf[48] = {0};
    int used_cnt = 0;
 
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
    memset(gateway, 0, size);
    for(; NLMSG_OK(nlmsg, len); nlmsg = NLMSG_NEXT(nlmsg, len)){
        memset(rt_info, 0, sizeof(struct route_info));
        parse_routes(nlmsg, rt_info, gateway_buf, ifname_buf);
        if (!strstr(gateway, gateway_buf)) {
            used_cnt += snprintf(gateway + used_cnt, size - 1, "%s ", gateway_buf);
        }
        if (used_cnt >= size) break;
    }
    gateway[used_cnt] = '\0';

    free(rt_info);
    close(sock);
   
    return gateway;
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
 * @brief get_net_mac 
 *
 * @param ip     [in]  target ip address
 * @param mac[6] [out] target mac address
 *
 * @return 0, if uscc; -1, if failed
 */
static unsigned char net_mac[6] = {0};
unsigned char *get_net_mac(char *dstip, int timeout)
{
    int fd, len;
    struct sockaddr addr;
    struct frame_arp snd_buf, recv_buf;
    char ifname_buf[128] = {0};
    char local_ip_buf[128] = {0};
    char *ifname = NULL;
    char *if_save = NULL;
    char *ip_save = NULL;
    unsigned char *src_mac = NULL;
    unsigned char src_ip[6];
    unsigned char dst_ip[6];
    char src_ip_buf[20];
    struct timeval tv = {0};
    fd_set set;
    struct timeval start = {0}, end = {0};
    int time_use = 0;

    memset(net_mac, 0, sizeof(net_mac));
    if (dstip == NULL) return net_mac;

    /**
     * check ifname
     */
    ifname = get_ifname(ifname_buf, sizeof(ifname_buf));
    if_save = strtok(ifname, " ");
    if (if_save == NULL) return net_mac;
    
    /**
     * local info
     */
    get_local_ip(AF_INET, if_save, local_ip_buf, sizeof(local_ip_buf));
    ip_save = strtok(local_ip_buf, " ");
    if (!ip_save) return NULL;
    ip2arr(src_ip_buf, src_ip);
    ip2arr(dstip, dst_ip);
    ip_save = strtok(local_ip_buf, " ");
    src_mac = get_mac(if_save, src_mac, sizeof(src_mac));
    if (!memcmp(dst_ip, src_ip, 4)) {
        return net_mac;
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
                memcpy(net_mac, recv_buf.ah.src_mac, 6);
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
    
    return net_mac;
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
        printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], 
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

