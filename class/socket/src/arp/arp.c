#include <arp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils/utils.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <property.h>
#include <socket_header.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>

typedef struct private_arp_t private_arp_t;
struct private_arp_t {
    /**
     * @brief public interface
     */
    arp_t public;

    /**
     * @brief fd of arp socket
     */
    int fd;

    /**
     * @brief property of socket
     */
    property_t *property;

    /**
     * @brief arp header
     */
    struct frame_arp hdr;

    /**
     * @brief interface name
     */
    char *ifname;

    /**
     * @brief socket addr
     */
    struct sockaddr addr;

    /**
     * @brief src ip address
     */
    unsigned char src_ip[4];

    /**
     * @brief src ip address
     */
    unsigned char dst_ip[4];
};

#define arp_fd       this->fd
#define arp_property this->property
#define arp_header   this->hdr
#define arp_ifname   this->ifname
#define arp_addr     this->addr
#define arp_msg      this->hdr
#define eth_dst_mac  this->hdr.fh.dst_mac 
#define eth_src_mac  this->hdr.fh.src_mac 
#define eth_protocol this->hdr.fh.protocol
#define arp_op       this->hdr.ah.ar_op
#define arp_src_mac  this->hdr.ah.src_mac 
#define arp_dst_mac  this->hdr.ah.dst_mac 
#define arp_src_ip   this->hdr.ah.src_ip 
#define arp_dst_ip   this->hdr.ah.dst_ip

METHOD(arp_t, open_, int, private_arp_t *this, char *ifname)
{
    char ifname_buf[128] = {0};
    char *save_str = NULL;

    if ((arp_fd = socket(AF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) <= 0) {
        perror("socket()");
        goto failed;
    }

    if (!ifname) {
        get_ifname(ifname_buf, sizeof(ifname_buf));
        ifname = strtok_r(ifname_buf, " ;,:", &save_str);
        if (!ifname) goto failed;
    }
    arp_ifname = strdup(ifname);

    memset(&arp_addr, 0, sizeof(arp_addr));
    arp_addr.sa_family = PF_PACKET;
    strncpy(arp_addr.sa_data, ifname, sizeof(arp_addr.sa_data));
    
    if (bind(arp_fd, (struct sockaddr *)&arp_addr, (socklen_t)sizeof(arp_addr)) < 0) {
        perror("bind()");
        goto failed;
    }

    arp_property = create_property(arp_fd);
    return arp_fd;

failed:
    if (arp_fd > 0) close(arp_fd);
    return -1;
}

METHOD(arp_t, send_, int, private_arp_t *this, arp_type_t type, char *dst_mac, char *src_mac, char *dst_ip, char *src_ip)
{
    char src_ip_buf[64] = {0};
    unsigned char un_src_ip[4] = {0};
    unsigned char un_dst_ip[4] = {0};
    unsigned char un_src_mac[6] = {0};
    unsigned char un_dst_mac[6] = {0};

    if (arp_fd < 1 || !dst_ip) return -1;

    /**
     * arp op
     */
    if (type == ARP_REQUEST) 
        arp_op = ARP_OP_REQUEST;
    else if (type == ARP_REPLY)
        arp_op = ARP_OP_REPLY;
    else return -1;

    /**
     * src mac address
     */
    if (!src_mac) {
        if (get_mac(arp_ifname, un_src_mac, sizeof(un_src_mac)) < 0)
            return -1;
    } else mac2arr(src_mac, un_src_mac);

    /**
     * src ip address
     */
    if (!src_ip) {
        if (get_local_ip(AF_INET, arp_ifname, src_ip_buf, sizeof(src_ip_buf)) < 0) 
            return -1;
        src_ip = src_ip_buf;
    }

    /**
     * transform address string info array
     */
    ip2arr(src_ip, un_src_ip);
    ip2arr(dst_ip, un_dst_ip);
    if (dst_mac) mac2arr(dst_mac, un_dst_mac);
    memcpy(this->src_ip, un_src_ip, sizeof(this->src_ip));
    memcpy(this->dst_ip, un_dst_ip, sizeof(this->dst_ip));

    /**
     * write into ether header
     */
    if (dst_mac) memcpy(eth_dst_mac, un_dst_mac, sizeof(eth_dst_mac));
    else memset(eth_dst_mac, -1, sizeof(eth_dst_mac));
    memcpy(eth_src_mac, un_src_mac, sizeof(eth_src_mac));

    /**
     * write into arp header
     */
    if (dst_mac) memcpy(arp_dst_mac, un_dst_mac, sizeof(arp_dst_mac));
    else memset(arp_dst_mac, 0, sizeof(arp_dst_mac));
    memcpy(arp_src_mac, un_src_mac, sizeof(arp_src_mac));
    memcpy(arp_dst_ip, un_dst_ip, sizeof(arp_dst_ip));
    memcpy(arp_src_ip, un_src_ip, sizeof(arp_src_ip));
    
    /**
     * send
     */
    return sendto(arp_fd, &arp_msg, sizeof(arp_msg), 0, &arp_addr, sizeof(arp_addr));
}

METHOD(arp_t, recv_, int, private_arp_t *this, arp_type_t type, void *buf, int size, int timeout_ms)
{
    int ret = 0;
    int recived = 0;
    fd_set fdset;
    socklen_t len = sizeof(arp_addr);
    struct timeval tv = {0};
    struct timeval cur = {0}, end = {0};
    struct frame_arp recv_buf;
    
    if (timeout_ms > 0) {
        gettimeofday(&end, NULL);
        end.tv_sec += timeout_ms / 1000 + (end.tv_usec + timeout_ms % 1000 * 1000) / 1000000 ;
        end.tv_usec = (end.tv_usec + timeout_ms % 1000 * 1000) % 1000000;
    }

    while (1) {
        FD_ZERO(&fdset);
        FD_SET(arp_fd, &fdset);

        tv.tv_usec = 0;
        tv.tv_sec = 1;
        ret = select(arp_fd + 1, &fdset, NULL, NULL, &tv);
        
        switch (ret) {
            case 0:
                break;
            case -1:
                break;
            default:
                if (FD_ISSET(arp_fd, &fdset)) {
                    ret = recvfrom(arp_fd, &recv_buf, sizeof(recv_buf), 0, &arp_addr, &len);
                    if (ret < FRAME_ARP_SIZE) break;

                    if (ntohs(recv_buf.fh.protocol) != 0x0806) break;
                    if (ntohs(recv_buf.ah.ar_op) != type) break;
                    if (memcmp(this->dst_ip, recv_buf.ah.src_ip, sizeof(this->dst_ip))) break;
                    print_mac(recv_buf.fh.src_mac, NULL);
                    if (buf) strncpy(buf, (void *)&recv_buf, size);
                    recived = 1;
                    break;
                }
                break;
        }

        if (timeout_ms > 0) {
            gettimeofday(&cur, NULL);
            if (cur.tv_sec > end.tv_sec || (cur.tv_sec == end.tv_sec && cur.tv_usec >= end.tv_usec)) {
                ret = -1;
                break;
            }
        }
        if (recived) break;
    }

    return ret;
}

METHOD(arp_t, close_, int, private_arp_t *this)
{
    return close(arp_fd);
}

METHOD(arp_t, destroy_, void, private_arp_t *this)
{
    if (arp_ifname) free(arp_ifname);
    if (arp_fd > 0) close(arp_fd);
    if (arp_property) arp_property->destroy(arp_property);
    free(this);
}

METHOD(arp_t, get_fd_, int, private_arp_t *this)
{
    return arp_fd;
}

arp_t *arp_create()
{
    private_arp_t *this;

    INIT(this, 
        .public = {
            .open    = _open_,
            .send    = _send_,
            .recv    = _recv_,
            .close   = _close_,
            .destroy = _destroy_,

            .get_fd  = _get_fd_,
        },
    );

    eth_protocol = htons(ETH_P_ARP);
    this->hdr.ah.ar_hrd = htons(ARPHRD_ETHER);
    this->hdr.ah.ar_pro = htons(ETH_P_IP);
    this->hdr.ah.ar_hln = 6;
    this->hdr.ah.ar_pln = 4;

    return &this->public;
}
