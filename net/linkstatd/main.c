#include <asm/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

int main(int argc, char* argv[]){
    struct sockaddr_nl sa;
    char msg[1024];
    struct ifinfomsg* ifi = NLMSG_DATA(msg);
    int fd;
    char ifname[64];

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK;

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    bind(fd, (struct sockaddr *) &sa, sizeof(sa));

    while(read(fd, msg, sizeof(msg))){
        printf("iface %s is %s\n", if_indextoname(ifi->ifi_index, ifname),
                ifi->ifi_flags & IFF_RUNNING ? "up":"down");
    }

    close(fd);

    return 0;
}
