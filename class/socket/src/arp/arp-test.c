#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arp.h>
#include <utils/get_args.h>
#include <utils/utils.h>

#define BOARD_MAC_ADDR "00:0E:8F:03:86:D0"
int main(int argc, char **argv)
{
    int ret = -1;
    int help_flag = 0;
    int board_mac = 0;
    struct options opt[] = {
        {"-m", "--mac",  0, RET_INT, ADDR_ADDR(board_mac)},
        {"-h", "--help", 0, RET_INT, ADDR_ADDR(help_flag)},
        {NULL}
    };
    struct usage usg[] = {
        {"-m, --mac",  "mac address"},
        {"-h, --help", "show usage"},
        {NULL},
    };
    char ip[20] = {0};
    
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
    return 0;

    arp_t *arp = arp_create();
    ret = arp->open(arp, "eth0");
    if (ret < 0) return -1;

    ret = arp->send(arp, ARP_REQUEST, NULL, NULL, "172.21.34.81", NULL);
    printf("ret: %d\n", ret);
    ret = arp->recv(arp, ARP_REPLY, NULL, 0, 1000);
    printf("ret: %d\n", ret);
    arp->destroy(arp);
    return 0;
}
