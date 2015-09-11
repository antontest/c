#ifndef __SOCKET_HEADER__
#define __SOCKET_HEADER__

#ifndef _LITTLE_ENDIAN_
#define _LITTLE_ENDIAN_ 0x01
#endif

#ifndef _BIG_ENDIAN_
#define _BIG_ENDIAN_ 0x02
#endif

#ifndef _BYTE_ORDER_ 
#define _BYTE_ORDER_ (0x0201 & 0xff)
#endif

/*
#if _BYTE_ORDER_ == _BIG_ENDIAN_
#error "big"
#elif _BYTE_ORDER_ == _LITTLE_ENDIAN_
#error "little"
#endif
*/
#ifndef ETHER_HDR
#define ETHER_HDR
struct ether_hdr {
    unsigned char dst_mac[6];
    unsigned char src_mac[6];
    unsigned short proto_type;
} __attribute__ ((__packed__));
#endif

#ifndef ICMP_HDR
#define ICMP_HDR
struct icmp_hdr {
    unsigned char i_type;
    unsigned char i_code;
    unsigned short i_cksum;
} __attribute__ ((__packed__));
#endif

#ifndef PING_HDR
#define PING_HDR
struct ping_hdr {
    unsigned char i_id;
    unsigned char i_seq;
} __attribute__ ((__packed__));
#endif

#ifndef FRAME_HDR
#define FRAME_HDR
struct frame_hdr {
        unsigned char dst_mac[6];
        unsigned char src_mac[6];
        unsigned short proto_type;
};
#endif

#ifndef ARP_HDR
#define ARP_HDR
struct arp_hdr
{
    unsigned short int ar_hrd;		/* Format of hardware address.  */
    unsigned short int ar_pro;		/* Format of protocol address.  */
    unsigned char ar_hln;		/* Length of hardware address.  */
    unsigned char ar_pln;		/* Length of protocol address.  */
    unsigned short int ar_op;		/* ARP opcode (command).  */
};
#endif

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1
#endif
#ifndef __BYTE_ORDER
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

struct ip_hdr
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char ihl:4;
    unsigned char version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned char version:4;
    unsigned char ihl:4;
#else
    #error "Please fix <bits/endian.h>" ;
#endif
    unsigned char tos;
    unsigned short tot_len;
    unsigned short id;
    unsigned short frag_off;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short check;
};

#ifndef FRAME_ARP
#define FRAME_ARP
//all frame 14+8+20 bytes
struct frame_arp {
    struct frame_hdr fh;
    struct arp_hdr ah;
    unsigned char src_mac[6];
    unsigned char src_ip[4];
    unsigned char dst_mac[6];
    unsigned char dst_ip[4];
};
#endif

struct frame_ip {
    struct frame_hdr fh;
    struct ip_hdr ih;
    unsigned char src_ip[4];
    unsigned char dst_ip[4];
};


#endif
