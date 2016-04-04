#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arp/arp.h>
#include <get_args/get_args.h>
#include <utils/utils.h>

#define BOARD_MAC_ADDR "00:0E:8F:03:86:D0"
int main(int argc, char **argv)
{
    int ret          = -1;
    int help_flag    = 0;
    int req_rep_flag = 0; /* req: 0, rep: 1 */
    int board_mac    = 0;
    char *mac        = NULL;
    char ip[20]      = {0};
    char *src_ip     = NULL;
    char *dst_ip     = NULL;
    char *ether_dev  = NULL;
    struct options opt[] = {
        {"-b", "--board_ip", 0, RET_INT, ADDR_ADDR(board_mac)},
        {"-s", "--src_ip",   1, RET_STR, ADDR_ADDR(src_ip)},
        {"-d", "--dst_ip",   1, RET_STR, ADDR_ADDR(dst_ip)},
        {"-e", "--ether",    1, RET_STR, ADDR_ADDR(ether_dev)},
        {"-m", "--mac",      1, RET_STR, ADDR_ADDR(mac)},
        {NULL, "--reply",    0, RET_INT, ADDR_ADDR(req_rep_flag)},
        {"-h", "--help",     0, RET_INT, ADDR_ADDR(help_flag)},
        {NULL}
    };
    struct usage usg[] = {
        {"-b, --board_ip", "get board ip address"},
        {"-s, --src_ip",   "src ip address"},
        {"-d, --dst_ip",   "dst ip address"},
        {"-m, --mac",      "mac address"},
        {"-h, --help",     "show usage"},
        {NULL},
    };
    arp_t *arp = NULL;
    
    get_args(argc, argv, opt);
    if (help_flag) {
        print_usage(usg);
        return 0;
    }
    
    if (board_mac) {
        get_remote_ip_by_mac(BOARD_MAC_ADDR, ip, sizeof(ip), 0);
        printf("%s\n", ip);
        return 0;
    }
    
    if (!dst_ip) return -1;
    arp = arp_create();
    if (!ether_dev) ether_dev = "eth0";
    ret = arp->open(arp, ether_dev);
    if (ret < 0) return -1;

    if (req_rep_flag) {
        ret = arp->send(arp, ARP_REPLY, mac, NULL, dst_ip, src_ip);
    }
    else
        ret = arp->send(arp, ARP_REQUEST, NULL, NULL, dst_ip, src_ip);
    printf("sended: %d\n", ret);
    //ret = arp->recv(arp, ARP_REPLY, NULL, 0, 1000);
    //printf("ret: %d\n", ret);
    arp->destroy(arp);
    return 0;
}
