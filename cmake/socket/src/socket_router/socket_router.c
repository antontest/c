/**
 * usual head files
 */
#include "socket_router.h"

int router(char *srcip, char *dstip, char *gateway)
{
    int fd, len;
    struct sockaddr addr;
    struct frame_arp snd_buf, recv_buf;
    char ifname[64];
    char *if_save = NULL;
    unsigned char src_mac[6];
    unsigned char gw_mac[6];
    unsigned char src_ip[6];
    unsigned char dst_ip[6];
    char src_ip_buf[20];
    struct timeval tv = {0};
    fd_set set;

    if (dstip == NULL) return -1;

    /**
     * check ifname
     */
    get_ifname(ifname);
    if_save = strtok(ifname, " ");
    if (if_save == NULL) return -1;
    
    /**
     * local info
     */
    get_ip_by_ifname(if_save, src_ip_buf);
    ip2arr(src_ip_buf, src_ip);
    ip2arr(dstip, dst_ip);
    get_mac_addr(if_save, src_mac);
    memset(gw_mac, -1, 6);

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
    memcpy(snd_buf.src_mac, src_mac, 6);
    memset(snd_buf.fh.dst_mac, -1, 6);
    memset(snd_buf.dst_mac, 0, 6);
    snd_buf.fh.proto_type = htons(ETH_P_ARP);
    snd_buf.ah.ar_hrd = htons(ARPHRD_ETHER);
    snd_buf.ah.ar_pro = htons(ETH_P_IP);
    snd_buf.ah.ar_hln = 6;
    snd_buf.ah.ar_pln = 4;
    snd_buf.ah.ar_op = htons(ARPOP_REQUEST);
    memcpy(snd_buf.dst_ip, dst_ip, 4);
    memcpy(snd_buf.src_ip, src_ip, 4);

    len = sizeof(struct sockaddr);
    sendto(fd, &snd_buf, sizeof(snd_buf), 0, &addr, len);
    FD_ZERO(&set);
    FD_SET(fd, &set);
    while (1) {
        tv.tv_sec = 0;
        tv.tv_usec = 1000 * 100;
        FD_ZERO(&set);
        FD_SET(fd, &set);

        select(fd + 1, &set, NULL, NULL, &tv);
        if (FD_ISSET(fd, &set))
        {
            recvfrom(fd, &recv_buf, sizeof(recv_buf), 0, NULL, NULL);
            if (!memcmp(recv_buf.src_ip, dst_ip, 4)) 
            {
                memcpy(recv_buf.fh.dst_mac, gw_mac, 6);
                memcpy(recv_buf.dst_mac, gw_mac, 6);
                sendto(fd, &recv_buf, sizeof(recv_buf), 0, &addr, len);
                break;
            }
        }
    }
    
    return 0;
}

int route(char *dstip)
{
    int fd, len;
    struct sockaddr addr;
    struct frame_ip recv_buf;
    char ifname[10] = {0};
    unsigned char src_mac[6];
    unsigned char gw_mac[6];
    unsigned char src_ip[6];
    unsigned char dst_ip[6];
    unsigned char gw_ip[6];
    char src_ip_buf[20] = {0};
    char gw_ip_buf[20] = {0};
    //char ip_buf[20] = {0};
    struct timeval tv = {0};
    fd_set set;

    printf("dstip: %s\n", dstip);
    if (dstip == NULL) return -1;

    /**
     * local info
     */
    get_gateway(gw_ip_buf, ifname);
    get_ip_by_ifname(ifname, src_ip_buf);
    get_mac_addr(ifname, src_mac);
    ip2arr(gw_ip_buf, gw_ip);
    ip2arr(src_ip_buf, src_ip);
    ip2arr(dstip, dst_ip);
    get_net_mac(gw_ip_buf, gw_mac, 0);

    /**
     * init addr
     */
    memset(&addr, 0, sizeof(addr));
    addr.sa_family = PF_PACKET;
    strcpy(addr.sa_data, ifname);

    /**
     * create socket
     */
    if ((fd = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ALL))) < 0) {
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

    print_ipv4(gw_ip, "gw_ip");
    print_ipv4(dst_ip, "dst_ip");
    print_mac(gw_mac, "gw_mac");
    //return 0;

    len = sizeof(struct sockaddr);
    FD_ZERO(&set);
    FD_SET(fd, &set);
    while (1) {
        tv.tv_sec = 0;
        tv.tv_usec = 1000 * 100;
        FD_ZERO(&set);
        FD_SET(fd, &set);

        select(fd + 1, &set, NULL, NULL, &tv);
        if (FD_ISSET(fd, &set))
        {
            recvfrom(fd, &recv_buf, sizeof(recv_buf), 0, NULL, NULL);
            if (ntohs(recv_buf.fh.proto_type) == 0x0806) continue;

            if (!memcmp(recv_buf.src_ip, dst_ip, 4)) 
            {
                /*
                printf("package coming. type: %04x\n", ntohs(recv_buf.fh.proto_type));
                print_mac(recv_buf.fh.src_mac, "src_mac");
                print_ipv4(recv_buf.src_ip, "src_ip");
                print_ipv4(recv_buf.dst_ip, "dst_ip");
                printf("ip protocol: %d\n", recv_buf.ih.protocol);
                printf("ip len: %d\n", ntohs(recv_buf.ih.tot_len));
                */
                memcpy(recv_buf.fh.dst_mac, gw_mac, 6);
                //memcpy(recv_buf.dst_ip, gw_ip, 4);
                print_ipv4(recv_buf.src_ip, "pakcage from: ");
                sendto(fd, &recv_buf, sizeof(recv_buf), 0, &addr, len);
            }
        }
    }
    
    return 0;
}

