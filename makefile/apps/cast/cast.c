#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils/utils.h>
#include <cast/cast.h>
#include <get_args/get_args.h>

int main(int argc, char **argv)
{
    int ret        = 0;
    int help_flag  = 0;
    int multi_flag = 0;
    int broad_flag = 0;
    int send_flag  = 0;
    int recv_flag  = 0;
    int send_times = 1;
    int port       = 5001;
    char buf[128]  = {0};
    char *ip       = "255.255.255.255";
    char *message  = NULL;
    struct options opt[] = {
        {"-h", "--help",      0, RET_INT, ADDR_ADDR(help_flag)},
        {"-s", "--send",      0, RET_INT, ADDR_ADDR(send_flag)},
        {"-r", "--recv",      0, RET_INT, ADDR_ADDR(recv_flag)},
        {"-i", "--ip",        1, RET_STR, ADDR_ADDR(ip)},
        {"-p", "--port",      1, RET_INT, ADDR_ADDR(port)},
        {"-m", "--multicast", 0, RET_INT, ADDR_ADDR(multi_flag)},
        {"-b", "--broadcast", 0, RET_INT, ADDR_ADDR(broad_flag)},
        {"-t", "--times",     1, RET_INT, ADDR_ADDR(send_times)},
        {"-e", "--message",   1, RET_STR, ADDR_ADDR(message)},
        {NULL}
    };
    struct usage usg[] = {
        {"-h, --help",      "show usage"},
        {"-i, --ip",        "ip address of casting"},
        {"-p, --port",      "port of casting"},
        {"-m, --multicast", "multi cast"},
        {"-b, --broadcast", "broad cast"},
        {"-t, --times",     "cast message sending times"},
        {"-e, --message",   "message of casting"},
        {NULL}
    };
    cast_t *cast = NULL;

    /**
     * parser command line 
     */
    get_args(argc, argv, opt);
    if (help_flag) {
        print_usage(usg);
        exit(0);
    }

    /**
     * arguements check
     */
    if (!multi_flag && !broad_flag) {
        fprintf(stderr, "Please choose one cast mode!\n");
        exit(1);
    } 
    if (multi_flag && !strcmp(ip, "255.255.255.255")) {
        fprintf(stderr, "Please give multicast ip address!\n");
        exit(1);
    }
    if (!send_flag && !recv_flag) {
        fprintf(stderr, "Please choose send mode or recv mode!\n");
        exit(1);
    }
    if (send_flag && !message) {
        fprintf(stderr, "Please input cast message!\n");
        exit(1);
    }
    
    /**
     * create cast instance
     */
    if (multi_flag) {
        cast = create_multicast(ip, port);
        //cast_t *cast = create_multicast("225.0.0.88", 5001);
    } else if (broad_flag) {
        cast = create_broadcast(ip, port);
        //cast = create_broadcast("255.255.255.255", port);
    }

    /**
     * cast sending or recving
     */
    if (send_flag) {
        while (send_times-- > 0) {
            ret = cast->send(cast, message, strlen(message));
            if (ret > 0) printf("cast send: %s succ.\n", message);
        }
    } else if (recv_flag) {
        while ((ret = cast->recv(cast, buf, sizeof(buf))) != 0) {
            if (ret > 0) {
                printf("cast recv: %s\n", buf);
            }
        }
    }
    cast->destroy(cast);

    return 0;
}
