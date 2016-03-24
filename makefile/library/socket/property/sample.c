#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <property.h>

int main(int argc, char **argv)
{
    char buf[128] = {0};
    property_t *p = create_property(-1);
    printf("ip: %s\n", get_local_ip(-1, NULL, buf, sizeof(buf)));
    printf("mac: %02x\n", get_mac("eth0", (unsigned char *)buf, sizeof(buf))[0]);
    printf("ifname: %s\n", get_ifname(buf, sizeof(buf)));
    printf("gateway: %s\n", get_gateway(buf, sizeof(buf)));
    //printf("ifname: %s\n", p->get_ifname(p, NULL, -1));
    p->destroy(p);
    return 0;
}
