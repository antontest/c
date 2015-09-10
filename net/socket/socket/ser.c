#include "socket_base.h"
#include "socket_app.h"

#define ipc_path "../ipc"

int eth_data_capture(int fd)
{
    char buf[ 5 * 1024] = {0};
    unsigned char dhost[6];
    unsigned char shost[6];
    unsigned short eth_type;
    unsigned char icmp_type;
    unsigned char icmp_code;
    socklen_t len = 0;

    while (1)
    {
        if (recvfrom(fd, buf, sizeof(buf), 0, NULL, &len) > 0)
        {
            parse_ether_head(buf, dhost, shost, &eth_type);
            printf("ether type: %d\n", eth_type);
            if (eth_type != 0x0800) continue;
            
            print_mac("dest mac", dhost);
            print_mac("dest mac", shost);
            parse_icmp_head(buf, &icmp_type, &icmp_code);
            printf("icmp type: %d\n", icmp_type);
            printf("icmp type: %d\n", icmp_code);
            if (icmp_type < 0 || icmp_type > 18) continue;
            if (icmp_code == 0) parse_ping(buf);

            sleep(1);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int fd = -1;
    int clifd = -1;
    int len = 0;
    char buf[128] = {0};
    struct socket_impl sck;

/*     if ((fd = raw_socket_init()) <= 0) return -1; */
/*     eth_data_capture(fd); */
/*      */
/*     return 0; */
    get_mac_addr("eth8", buf);
    printf("eth8 max rate: %dMb/s\n", get_eth_speed("eth8"));
    if (strlen(buf)) printf("mac: %s\n", buf);
    
    if ((fd = inet_server_create(&sck, SOCK_STREAM, NULL, 
                    5001)) == -1)
        return -1;

    clifd = socket_accept(fd);
    if (clifd > 0) printf("accept succ\n");
    else {
        printf("accept failed.\n");
        return -1;
    }
    while ((len = socket_recv(clifd, buf, sizeof(buf))))
    {
        if (len <= 0) break;
        printf("len: %d, info: %s\n", len, buf);

        socket_send(clifd, "hi", 3);
    }

    return 0;
}
