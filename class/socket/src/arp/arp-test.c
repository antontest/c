#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arp.h>

int main(int argc, char **argv)
{
    int ret = -1;
    arp_t *arp = arp_create();

    ret = arp->open(arp, "eth1");
    if (ret < 0) return -1;

    ret = arp->send(arp, ARP_REQUEST, NULL, NULL, "172.21.34.9", NULL);
    printf("ret: %d\n", ret);
    ret = arp->recv(arp, ARP_REPLY, NULL, 0, 1000);
    printf("ret: %d\n", ret);
    arp->destroy(arp);
    return 0;
}
