#ifndef __SOCKET_HEADER__
#define __SOCKET_HEADER__
#include <netinet/in.h>

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 0x01
#endif

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 0x02
#endif

#ifndef __BYTE_ORDER 
#define __BYTE_ORDER (0x0201 & 0xff)
#endif

/*
#if _BYTE_ORDER_ == __BIG_ENDIAN
#error "big"
#elif _BYTE_ORDER_ == __LITTLE_ENDIAN
#error "little"
#endif
*/

// 14 bytes
struct ether_hdr {
    unsigned char   dst_mac[6];
    unsigned char   src_mac[6];
    unsigned short  protocol;
};

#ifndef ARP_HDR
#define ARP_HDR
// 28 bytes
struct arp_hdr
{
    unsigned short  ar_hrd;		/* Format of hardware address.  */
    unsigned short  ar_pro;		/* Format of protocol address.  */
    unsigned char   ar_hln;		/* Length of hardware address.  */
    unsigned char   ar_pln;		/* Length of protocol address.  */
    unsigned short  ar_op;		/* ARP opcode (command).  */
    unsigned char   src_mac[6];
    unsigned char   src_ip[4];
    unsigned char   dst_mac[6];
    unsigned char   dst_ip[4];
};
#define ARP_HEADER_SIZE sizeof(struct arp_dhr)
#endif

#ifndef IP_HEADER
#define IP_HEADER
// 20 bytes
struct ip_hdr
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char   ihl:4;
    unsigned char   version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned char   version:4;
    unsigned char   ihl:4;
#else
    #error "Please fix <bits/endian.h>" ;
#endif
    unsigned char   tos;
    unsigned short  tot_len;
    unsigned short  id;
    unsigned short  frag_off;
    unsigned char   ttl;
    unsigned char   protocol;
    unsigned short  check;
    unsigned char   src_ip[4];
    unsigned char   dst_ip[4];
};
#define IP_HEADER_SIZE sizeof(struct ip_hdr)
#endif

#ifndef UDP_HEADER
#define UDP_HEADER
// 8 bytes
struct udp_hdr {
    unsigned short  src_port;
    unsigned short  dst_port;
    unsigned short  len;
    unsigned short  check;
};
#define UDP_HEADER_SIZE sizeof(struct udp_hdr)
#endif

#ifndef TCP_HEADER
#define TCP_HEADER
// 20 bytes
struct tcp_hdr {
    unsigned short  src_port;
    unsigned short  dst_port;
    unsigned long   seq;
    unsigned long   ack;
    unsigned char   off;
    unsigned char   flags;
    unsigned short  win;
    unsigned short  check;
    unsigned short  urg;
};
#define TCP_HEADER_SIZE sizeof(struct tcp_hdr)
#endif

#ifndef ICMP_HEADER
#define ICMP_HEADER
struct icmp_hdr {
    unsigned char   type;
    unsigned char   code;
    unsigned short  check;
    union
    {
        unsigned char   pptr;
        struct in_addr  gwaddr;
        struct idseq {
            unsigned short id;
            unsigned short seq;
        } idseq;
        unsigned long   ih_void;

        struct pmtu {
            unsigned short ipm_void;
            unsigned short ipm_nextmtu;
        } pmtu;

        struct rtradv {
            unsigned char   rt_num_addr;
            unsigned char   rt_wpa;
            unsigned short  rt_lifetime;
        } rtradv;
    } icmp_hun;
};
#define ICMP_HEADER_SIZE sizeof(struct icmp_hdr)
#endif

#ifndef FRAME_ARP
#define FRAME_ARP
//all frame 14+8+20 bytes
struct frame_arp {
    struct ether_hdr fh;
    struct arp_hdr   ah;
};
#define FRAME_ARP_SIZE sizeof(struct frame_arp)
#endif

#ifndef FRAME_IP
#define FRAME_IP
struct frame_ip {
    struct ether_hdr fh;
    struct ip_hdr    ih;
    union {
        struct udp_hdr uh;
        struct tcp_hdr th;
    } ipproto;
    void *data;
};
#endif

#ifndef FRAME_UDP
#define FRAME_UDP
//all frame 14+20+8 bytes
struct frame_udp {
    struct ether_hdr fh;
    struct ip_hdr    ih;
    struct udp_hdr   uh;
};
#define FRAME_UDP_SIZE sizeof(struct frame_udp)
#endif

#ifndef FRAME_TCP
#define FRAME_TCP
//all frame 14+20+20 bytes
struct frame_tcp {
    struct ether_hdr fh;
    struct ip_hdr    ih;
    struct tcp_hdr   th;
};
#define FRAME_TCP_SIZE sizeof(struct frame_tcp)
#endif

#define ETHER_HEADER_SIZE sizeof(struct frame_dhr)
unsigned short check_sum(unsigned short *buf, int nwords);

#endif

