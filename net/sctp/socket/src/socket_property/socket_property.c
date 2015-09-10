#include "socket_property.h"

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
int get_ip(char ip[])
{
    struct ifaddrs *ifaddr = NULL;
    void *tmp_addr = NULL;
    char tmp_ip[20] = {0};
    char *hname = NULL;

    if (ip == NULL) return -1;

    strcpy(ip, "\0");
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
 * @brief get ip address by ifname 
 *
 * @param ifname [in]  interface name
 * @param ip     [out] ip address 
 *
 * @return 0, if succ; -1, if failed
 */
int get_ip_by_ifname(const char *ifname, char *ip)
{
    int fd = -1;
    struct ifreq ifr;
    struct sockaddr_in addr = {0};
    if (ifname == NULL) return -1;

    strcpy(ifr.ifr_name, ifname);
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    strcpy(ip, "\0");
    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) 
    {
        memcpy(&addr, &ifr.ifr_addr, sizeof(ifr.ifr_addr));
        strcpy(ip, inet_ntoa(addr.sin_addr));
        close(fd);
        return 0;
    }

    close(fd);

    return -1;
}

/**
 * @brief get hardware address by interface name
 *
 * @param ifname [in]  interface name
 * @param mac    [out] hardware addree
 *
 * @return 0, if succ; -1, if failed
 */
int get_mac_addr(const char *ifname, char *mac)
{
    int fd = -1;
    struct ifreq ifreq;

    if (mac == NULL) return -1;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    memset(&ifreq, 0, sizeof(ifreq));
    strcpy(ifreq.ifr_name, ifname);

    if (ioctl(fd, SIOCGIFHWADDR, &ifreq) < 0)
        return -1;
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char) ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[5]
            );

    return 0;
}

/**
 a* @brief get subnet ip address 
 *
 * @param ip   [in] ip
 * @param mask [in] mask
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

    return strdup(inet_ntoa(subnet));
}

/**
 * @brief convert mask to bits of mask
 *
 * @param mask [in] mask address
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
    {
        perror("fcntl F_GETFL");
        return -1;
    }

    if (fcntl(fd, F_SETFL, flag & ~O_NONBLOCK) < 0)
    {
        perror("fcntl F_SETFL");
        return -1;
    }

    return 0;
}

/**
 * @brief make listen socket reuseable 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if fail
 */
int make_listen_socket_reuseable(int fd)
{
    int on = 1;

    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
}

/**
 * @brief make socket keep alive 
 *
 * @param fd [in] socket fd
 *
 * @return 0, if succ; -1, if fail
 */
/*int make_socket_keep_alive(int fd)
{
    int on = 1;

    return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));
}*/

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
    {
        perror("fcntl F_GETFD");
        return -1;
    }

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
    {
        perror("fcntl F_SETFD");
        return -1;
    }

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
    int keep_idle = 60;     /* if connection no data exchanges in \
				60 seconds, then detect */
    int keep_interval = 5;  /* when detecting, the time interval is 5 seconds */
    int keep_count = 3;     /* The number of attempts to detect. \
				If the first probe packet is received, then the 2 time no longer. */

    /**
     * open keepalive mechanism
     */
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, \
                &keep_alive, sizeof(keep_alive)) == -1)
    {
        perror("set keep alive");
        return -1;
    }

    /* Default settings are more or less garbage, with the keepalive time 
     * set to 7200 by default on Linux. Modify settings to make the feature 
     * actually useful. */  
  
    /* Send first probe after interval. */  
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, \
                &keep_idle, sizeof(keep_idle)) == -1)
    {
        perror("set keep idle");
        return -1;
    }

    /* Send next probes after the specified interval. Note that we set the 
     * delay as interval / 3, as we send three probes before detecting 
     * an error (see the next setsockopt call). */  
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, \
                &keep_interval, sizeof(keep_interval)) == -1)
    {
        perror("set keep interval");
        return -1;
    }

    /* Consider the socket in error state after three we send three ACK 
     * probes without getting a reply. */  
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, \
                &keep_count, sizeof(keep_count)) == -1)
    {
        perror("set keep idle");
        return -1;
    }

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
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size)) < 0)
    {
        perror("set recv buf size");
        return -1;
    }

    return 0;
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
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size)) < 0)
    {
        perror("set send buf size");
        return -1;
    }

    return 0;
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

    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buf_size, \
                (socklen_t *)&len) < 0)
    {
        perror("get recv buf size");
        return -1;
    }

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

    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buf_size, \
                (socklen_t *)&len) < 0)
    {
        perror("get send buf size");
        return -1;
    }

    return buf_size;
}

/**
 * @brief set socket close action 
 *
 * @param fd    [in] socket fd
 * @param is_on [in] swith of close action
 * @param tm_s  [in] time
 *
 * @return 0, if succ; -1, if failed
 */
int make_socket_close_action(int fd, int is_on, int tm_s)
{
    struct linger so_linger = {0};

    so_linger.l_onoff = is_on;
    so_linger.l_linger = tm_s;

    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, \
                &so_linger, sizeof(so_linger)))
    {
        perror("setsockopt so_linger");
        return -1;
    }

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
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, \
                &on, sizeof(on)))
    {
        perror("setsockopt so_broadcast");
        return -1;
    }

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
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, \
                &on, sizeof(on)))
    {
        perror("setsockopt so_multicast_loop");
        return -1;
    }

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
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, \
                &ttl, sizeof(ttl)))
    {
        perror("setsockopt so_multicast_ttl");
        return -1;
    }

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
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, \
                mrq, sizeof(struct ip_mreq)))
    {
        perror("setsockopt so_multicast_ttl");
        return -1;
    }

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
    if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, \
                mrq, sizeof(struct ip_mreq)))
    {
        perror("setsockopt so_multicast_ttl");
        return -1;
    }

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
    //int tv = -1;
    int len = sizeof(tv);

    if (getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, \
                &tv, (socklen_t *)&len))
    {
        perror("getsockopt so_sndtimeo");
        return -1;
    }

    //return tv;
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

    if (getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, \
                &tv, (socklen_t *)&len))
    {
        perror("getsockopt so_rcvtimeo");
        return -1;
    }

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
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, \
                &tv, len))
    {
        perror("setsockopt so_sndtimeo");
        return -1;
    }

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
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, \
                &tv, len))
    {
        perror("setsockopt so_rcvtimeo");
        return -1;
    }

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

    if (getsockopt(fd, SOL_SOCKET, SO_PROTOCOL, \
                &type, (socklen_t *)&len))
    {
        perror("getsockopt so_protocol");
        return -1;
    }

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

    if (getsockopt(fd, SOL_SOCKET, SO_TYPE, \
                &type, (socklen_t *)&len))
    {
        perror("getsockopt so_type");
        return -1;
    }

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
    {
        perror("ioctl get interface flags");
        return -1;
    }

    if (on) req.ifr_flags |= IFF_PROMISC;
    else req.ifr_flags &= ~IFF_PROMISC;

    if (ioctl(fd, SIOCSIFFLAGS, &req) < 0)
    {
        perror("ioctl set interface flags");
        return -1;
    }
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
    {
        perror("get interface index");
        return -1;
    }

    return 0;
}

/**
 * @brief get interface name 
 *
 * @param ifname [out] interface name
 *
 * @return 0, if succ; -1, if failed.
 */
int get_ifname(char *ifname)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[2048];

    if (ifname == NULL) return -1;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        printf("socket error\n");
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) 
    {
        printf("ioctl error\n");
        return -1;
    }

    int count = 0;
    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    
    strcpy(ifname, "\0");
    for (; it != end; ++it) 
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) 
        {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) 
            { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) 
                {
                    count ++ ;
		            strcat(ifname, ifr.ifr_name);
                    strcat(ifname, " ");
                }
            }
        }
        else
        {
            printf("get mac info error\n");
            return -1;
        }
    }

    return 0;
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
        {
            printf("big endian\n");
            return 1;
        }
        else if (un.c[0] == 0x02 && un.c[1] == 0x01)
        {
            printf("little endian\n");
            return 0;
        }
        else 
        {
            printf("unknown\n");
            return -1;
        }
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
