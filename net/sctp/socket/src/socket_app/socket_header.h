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

struct ether_hdr {
    unsigned char ether_dhost[6];
    unsigned char ether_shost[6];
    unsigned short proto_type;
} __attribute__ ((__packed__));

struct icmp_hdr {
    unsigned char i_type;
    unsigned char i_code;
    unsigned short i_cksum;
} __attribute__ ((__packed__));

struct ping_hdr {
    unsigned char i_id;
    unsigned char i_seq;
} __attribute__ ((__packed__));


#endif
