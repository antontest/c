#include <udp/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils/utils.h>
#include <get_args/get_args.h>

int main(int argc, char **argv)
{
    int ret = 0;
    char buf[64] = {0};
    char *remote_ip = NULL;
    char *ip        = NULL;
    char *msg       = NULL;
    int times       = 1;
    int timeout     = 0;
    int help_flag   = 0;
    int port        = 0;
    int server_flag = 0;
    int client_flag = 0;
    udp_t *udp      = NULL;
    struct options opt[] = {
        {"-r", "--remote_ip", 1, RET_STR, ADDR_ADDR(remote_ip)},
        {"-i", "--ip",        1, RET_STR, ADDR_ADDR(ip)},
        {"-p", "--port",      1, RET_INT, ADDR_ADDR(port)},
        {"-s", "--server",    0, RET_INT, ADDR_ADDR(server_flag)},
        {"-c", "--client",    0, RET_INT, ADDR_ADDR(client_flag)},
        {"-m", "--message",   1, RET_STR, ADDR_ADDR(msg)},
        {"-t", "--times",     1, RET_INT, ADDR_ADDR(times)},
        {"-w", "--timeout",   1, RET_INT, ADDR_ADDR(timeout)},
        {"-h", "--help",      0, RET_INT, ADDR_ADDR(help_flag)},
        {NULL},
    };
    struct usage usg[] = {
        {"-r, --remote_ip", "remote ip address"},
        {"-i, --ip",        "ip address"},
        {"-p, --port",      "port"},
        {"-s, --server",    "flag of server"},
        {"-c, --client",    "flag of client"},
        {"-m, --message",   "message sending to server"},
        {"-t, --times",     "times of message sending"},
        {"-w, --timeout",   "timeout of connecting and message sending"},
        {"-h, --help",      "show usage"},
        {NULL},
    };
    
    get_args(argc, argv, opt);
    if (help_flag) {
        print_usage(usg);
        return 0;
    }
    if (!server_flag && !client_flag) return -1;
    if (!ip) ip = "%any";
    if (!port) port = 5001;

    udp = udp_create();
    ret = udp->socket(udp, AF_INET);
    if (ret < 0) goto over;

    if (client_flag) {
        /*
           ret = udp->connect(udp);
           if (ret < 0) goto over;
           ret = udp->send(udp, "hi", 2);
        */
        while (times-- > 0) {
            ret = udp->sendto(udp, msg, strlen(msg), ip, port);
            if (ret > 0) printf("send succ: %s\n", msg);
            if (times > 0 && timeout > 0) usleep(1000 * timeout);
        }
    } else {
        ret = udp->bind(udp, ip, port);
        if (ret < 0) goto over;

        if (times <= 0) {
            times = 0x9999;   
        }
        while (times-- > 0) {
            ret = udp->recvfrom(udp, buf, sizeof(buf), remote_ip, 0);
            if (ret > 0) printf("%s\n", buf);
        }
    }


over:
    udp->destroy(udp);
    return 0;
}
