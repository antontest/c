#ifndef __SOCKET_ARP_H__
#define __SOCKET_ARP_H__

#ifndef __SOCKET_HEADER_H__
#define __SOCKET_HEADER_H__
#include "socket_header.h"
#endif
#include "socket_property.h"
#include <pthread.h>
#include <netpacket/packet.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/sctp.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <pcap.h>

struct pcap_info {
    int fd;
    unsigned char attack_ip[4];
    unsigned char attack_mac[6];
    unsigned char local_ip[4];
    unsigned char local_mac[6];
    char ifname[20];
    unsigned char gateway_ip[4];
    union  {
        struct in_addr in_addr;
        struct sockaddr addr;
        struct sockaddr_in addr_in;
        struct sockaddr_ll addr_ll;
    } addr;
    char* proto;
    pcap_handler cb;
    void* cb_arg;
};

/*************************************************************
**********  Function Declaration Of ARP Cheating  ************
**************************************************************/
/**
 * @brief arp_request_package 
 *
 * @param frame
 * @param src_ip[4]
 * @param src_mac[6]
 * @param dst_ip[4]
 */
void arp_request_package(struct frame_arp *frame, unsigned char src_ip[4], 
                    unsigned char src_mac[6], unsigned char dst_ip[4]);

/**
 * @brief arp_reply_package 
 *
 * @param frame
 * @param src_ip[4]
 * @param src_mac[6]
 * @param dst_ip[4]
 * @param dst_mac[6]
 */
void arp_reply_package(struct frame_arp *frame, unsigned char src_ip[4], 
                    unsigned char src_mac[6], unsigned char dst_ip[4], unsigned char dst_mac[6]);

/**
 * @brief arp_request_send 
 *
 * @param fd
 * @param addr
 * @param dst_ip[4]
 * @param src_ip[4]
 * @param src_mac[6]
 *
 * @return send count, if succ; -1, if failed 
 */
int arp_request_send(int fd, struct sockaddr *addr, unsigned char dst_ip[4], 
             unsigned char src_ip[4], unsigned char src_mac[6]);

/**
 * @brief arp_reply_send 
 *
 * @param fd
 * @param addr
 * @param dst_ip[4]
 * @param dst_mac[6]
 * @param src_ip[4]
 * @param src_mac[6]
 *
 * @return send count, if succ; -1, if failed 
 */
int arp_reply_send(int fd, struct sockaddr *addr, unsigned char dst_ip[4], unsigned char dst_mac[6], 
             unsigned char src_ip[4], unsigned char src_mac[6]);

/**
 * @brief pcap_listen 
 *
 * @param arg
 *
 * @return 
 */
void* pcap_listen(void *arg);

/**
 * @brief arp_cheat 
 *
 * @param ifname     [in] interface name
 * @param attack_ip  [in] ip address of attacking
 * @param gateway_ip [in] ip address of gateway
 *
 * @return 0, if succ; -1, if failed
 */
int arp_cheat(const char *ifname, const char *attack_ip, 
        const char *gateway_ip);


#endif
