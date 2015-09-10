/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "socket_app.h"
#include "socket_arp.h"

/*********************************************************
 *****************    Global Variable    *****************
 *********************************************************/
static char ip[256] = {0};
static unsigned short protocol = 0;
static unsigned short port = 5001;
static unsigned int sleep_time = -1;
static unsigned int times = 1;
static char message[1024] = {0};
static char proto_str[10] = {0};
static int ser_flag = 0;
static int cli_flag = 0;
static int interact_flag = 0;
static char ether[100]= {0};
static char gateway_ip[15]= {0};
static int mac_flag = 0;
static int eth_flag = 0;
static int cheat_flag = 0;
static int gateway_flag = 0;
static int netmac_flag = 0;

/*********************************************************
 *****************    Macro Defination    ****************
 *********************************************************/
enum PROTOCOL_TYPE {
    PROTO_TCP = 0   ,
    PROTO_UDP       ,
    PROTO_SCTP      ,
    PROTO_BROADCAST ,
    PROTO_MULTICAST
};

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
static void print_usage();
static int parser_args(int agrc, char *agrv[]);
int net_tcp_ser(const char *ip, const unsigned short port);
int net_tcp_cli(const char *ip, const unsigned short port);
int net_udp_ser(const char *ip, unsigned short port);
int net_udp_cli(const char *ip, unsigned short port);

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */

    /**
     * Get paramters from the command line
     */
    if (parser_args(agrc, agrv) < 0) {
        //print_usage();
        rt = -1;
        goto error;
    };


    /**
     * get net mac address
     */
    if (netmac_flag) {
        unsigned char buf[6];
        struct timeval start = {0}, end = {0};
        int tuse = 0;

        gettimeofday(&start, NULL);
        if (!get_net_mac(ip, buf)) {
            gettimeofday(&end, NULL);
            tuse = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            printf("\033[0;m%s MAC Address: \033[0;35m%02x:%02x:%02x:%02x:%02x:%02x\033[0m, used %d.%03ds.\n", ip, 
                    buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], (int)tuse/ 1000000, (int)(tuse % 1000000));
        } else {
            printf("\033[0;31mget %s net mac failed\n\033[0m", ip);
            return -1;
        }
        
        return 0;
    }

    /**
     * get mac address
     */
    if (mac_flag) {
        if (!strlen(ether)) get_ifname(ether);
        unsigned char buf[6] = {0};
        char *str = NULL, *save_str = NULL;

        for (str = ether; ; str = NULL) {
            save_str = strtok(str, " ");
            if (save_str == NULL) return -1;

            if (!get_mac_addr(save_str, buf)) {
                printf("\033[0;32m%s: \033[0;35m%02x:%02x:%02x:%02x:%02x:%02x\n\033[0m", save_str, 
                        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
            } else {
                printf("\033[0;31m%s: -1\n\033[0m", save_str);
                return -1;
            }

            return 0;
        }
    }

    /**
     * aro cheating
     */
    if (cheat_flag) {
        if (!strlen(ip)) {
            printf("\033[0;31minput ipaddress when arp cheating\n\033[0m");
            return -1;
        }

        //if (!strlen(ether) || !strlen(gateway_ip)) 
        //    get_gateway(gateway_ip, ether);
        //rt = arp_cheat(ether, ip, gateway_ip);
        rt = arp_cheating(ip);
        
        return rt;
    }

    /**
     * get interface name
     */
    if (eth_flag && !strlen(ether)) {
        if (!get_ifname(ether)) {
            printf("\033[0;35m%s\n\033[0m", ether);
            return 0;
        } else {
            printf("\033[0;31mget interface failed\n\033[0m");
            return -1;
        }
    }

    /**
     * get gateway ip address
     */
    if (gateway_flag && !strlen(gateway_ip)) {
        if (!strlen(ether)) {
            get_ifname(ether);
        }
        if (!strlen(ether)) {
            printf("\033[0;31mget interface failed\n\033[0m");
            return -1;
        } 
        
        char *str = NULL, *save_str = NULL;
        for (str = ether; ; str = NULL) {
            save_str = strtok(str, " ");
            if (save_str == NULL) return -1;

            if (!get_gateway(gateway_ip, save_str)) {
                printf("\033[0;32m%s\'s gateway address is:\033[0;35m %s\n\033[0m", 
                        save_str, gateway_ip);
                return 0;
            } else {
                printf("\033[0;31mget interface failed\n\033[0m");
                return -1;
            }
        }
    }

    if (!strlen(ip)) strcpy(ip, "127.0.0.1");
    printf("\033[1;31mConfiguration: \n");
    printf("\033[1;35m  protocol    : \033[1;32m%s\n", proto_str);
    printf("\033[1;35m  ip          : \033[1;32m%s\n", ip);
    printf("\033[1;35m  port        : \033[1;32m%d\n", port);
    printf("\033[1;35m  ser         : \033[1;32m%d\n", ser_flag);
    printf("\033[1;35m  cli         : \033[1;32m%d\n", cli_flag);
    printf("\033[1;35m  times       : \033[1;32m%d\n", times);
    printf("\033[1;35m  wait  time  : \033[1;32m%d\n", sleep_time);
    printf("\033[0m\n");

    switch (protocol)
    {
        case PROTO_TCP:
            if (ser_flag) net_tcp_ser(ip, port);
            else if (cli_flag) net_tcp_cli(ip, port);
            break;
        case PROTO_UDP:
            if (ser_flag) net_udp_ser(ip, port);
            else if (cli_flag) net_udp_cli(ip, port);
            break;
        case PROTO_BROADCAST:
            if (ser_flag) udp_broadcast_recv(ip, port, times, NULL, 0);
            else if (cli_flag)
            {
                if (strlen(message))
                    udp_broadcast_send(ip, port, times, message);
                else 
                    udp_broadcast_send(ip, port, times, NULL);
            }
        case PROTO_MULTICAST:
            if (ser_flag) udp_multicast_recv(ip, port, times, NULL, 0);
            else if (cli_flag)
            {
                if (strlen(message))
                    udp_multicast_send(ip, port, times, message);
                else 
                    udp_multicast_send(ip, port, times, NULL);
            }
            break;
        case '?':
        default:
            break;
    }

error:
    /**
     * error handling
     */

    return rt;
}

void on_accept(int fd, void *arg)
{
    printf("accepted from %s\n", \
            inet_ntoa(((struct socket_impl *)arg)->addr.in_addr.sin_addr));
}

void on_connect(int fd, void *arg)
{
    printf("connected %s succ\n", \
            inet_ntoa(((struct socket_impl *)arg)->addr.in_addr.sin_addr));
}

void on_recv(int fd, void *arg)
{
    char buf[128] = {0};
    int sock_type = get_socket_type(fd);
    int proto_type = get_socket_protocol(fd);

    switch (sock_type) {
        case SOCK_STREAM:
            if (proto_type == IPPROTO_TCP)
                socket_recv(fd, buf, sizeof(buf));
            else if (proto_type == IPPROTO_SCTP) ;
            else return; 
            break;
        case SOCK_DGRAM:
            socket_addr_recvfrom(fd, buf, sizeof(buf), 
                    (void *)&((struct socket_impl *)arg)->addr.in_addr);
            break;
        default:
            return;
            break;
    }

    printf("recv from %s: %s\n", 
            inet_ntoa(((struct socket_impl *)arg)->addr.in_addr.sin_addr), 
            buf);
}

void on_close(int fd, void *arg)
{
    if (cli_flag) printf("server %s closed.\n", \
            inet_ntoa(((struct socket_impl *)arg)->addr.in_addr.sin_addr));
    else printf("client %s closed\n", \
            inet_ntoa(((struct socket_impl *)arg)->addr.in_addr.sin_addr));
}

/**
 * @brief create tcp server
 *
 * @param ip   [in] ip address of tcp server
 * @param port [in] listen port of tcp server
 *
 * @return 0, if succ; -1, if failed
 */
int net_tcp_ser(const char *ip, const unsigned short port)
{
    struct socket_impl sck = {0};
    char buf[1024] = {0};
    int ret = -1;

    if (inet_server_create(&sck, SOCK_STREAM, ip, port) < 0)
    {
        printf("tcp server create failed\n");
        return -1;
    }

    socket_event_add(&sck.evl, SOCKET_ON_ACCEPT, on_accept, &sck);
    socket_event_add(&sck.evl, SOCKET_ON_RECV, on_recv, &sck);
    socket_event_add(&sck.evl, SOCKET_ON_CLOSE, on_close, &sck);
    printf("tcp server create succ\n");
    
    if (interact_flag)
    {
        while (1)
        {
            printf("Please input: \n");
            if (gets(buf) == NULL) break;
            if (!strcasecmp("end", buf)) break;
            if (strlen(buf) && sck.curr_cli_fd > 0) 
                ret = socket_send(sck.curr_cli_fd, buf, strlen(buf));
            if (ret > 0)
                printf("send: %s\n", buf);
            ret = -1;
        }
    }

    if (sleep_time <= 0) while (1);
    else sleep(sleep_time);

    socket_close(sck.fd);

    return 0;
}

/**
 * @brief create tcp server
 *
 * @param ip   [in] ip address of tcp server
 * @param port [in] listen port of tcp server
 *
 * @return 0, if succ; -1, if failed
 */
int net_tcp_cli(const char *ip, const unsigned short port)
{
    struct socket_impl sck = {0};
    char buf[1024] = {0};
    int ret = -1;

    socket_event_add(&sck.evl, SOCKET_ON_CONNECT, on_connect, &sck);
    socket_event_add(&sck.evl, SOCKET_ON_CLOSE, on_close, &sck);
    socket_event_add(&sck.evl, SOCKET_ON_RECV, on_recv, &sck);

    if (inet_client_connect(&sck, SOCK_STREAM, ip, port) < 0)
        return -1;
    
    if (strlen(message))
    {
        while (times-- > 0)
        {
            if (socket_send(sck.fd, message, strlen(message)) > 0)
                printf("send to %s: %s\n", ip, message);
            sleep(1);
        }
    }

    if (interact_flag)
    {
        while (1)
        {
            printf("Please input: \n");
            if (gets(buf) == NULL) break;
            if (!strcasecmp("end", buf)) break;
            if (strlen(buf)) 
                ret = socket_send(sck.fd, buf, strlen(buf));
            if (ret > 0)
                printf("send: %s\n", buf);
            ret = -1;
        }
    }

    if (sleep_time > 0) sleep(sleep_time);

    socket_close(sck.fd);

    return 0;
}

int net_udp_ser(const char *ip, unsigned short port)
{
    struct socket_impl sck = {0};
    int ret = -1;

    if (inet_server_create(&sck, SOCK_DGRAM, ip, port) < 0)
    {
        printf("udp server create failed\n");
        return -1;
    }

    socket_event_add(&sck.evl, SOCKET_ON_RECV, on_recv, &sck);
    printf("udp server create succ\n");
    
    if (sleep_time <= 0) while (1);
    else sleep(sleep_time);

    socket_close(sck.fd);

    return ret; 
}

int net_udp_cli(const char *ip, unsigned short port)
{
    struct socket_impl sck = {0};
    int ret = -1;

    if (inet_client_connect(&sck, SOCK_DGRAM, ip, port) < 0)
    {
        printf("udp server create failed\n");
        return -1;
    }

    socket_event_add(&sck.evl, SOCKET_ON_RECV, on_recv, &sck);
    printf("udp server create succ\n");
    
    if (strlen(message))
    {
        while (times-- > 0)
        {
            if (socket_addr_sendto(sck.fd, message, strlen(message), (void *)&sck.addr.in_addr) > 0)
                printf("send to %s: %s\n", ip, message);
            sleep(1);
        }
    }

    if (sleep_time > 0) sleep(sleep_time);

    socket_close(sck.fd);

    return ret; 
}

/**
 * @brief print usage of the pragram 
 */
static void print_usage() 
{
    printf("\033[0;31m/********************Program Usage***********************/\033[0m\n");  
    printf("\033[0;31mName  : \033[0;32mSocket\033[0m\n");  
    printf("\033[0;31mVers  : \033[0;32m1.0.0\033[0m\n");
    printf("\033[0;31mTime  : \033[0;32m2015.06.04\033[0m\n");
    printf("\033[0;31mBrief : \033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-c|--client     -s|--server\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-a|--agreement  <[t|tcp|u|udp|s|sctp|b|broadcast|m|multicase]>\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-i|--ip         <ip address>    -p|--port <port>\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-t|--times      <message_send_times> -w|--wait <wait time>\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-m|--message    <message_to_send>\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-e|--ether      <interface_name>\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-q|--interact   --whether interact to each other\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-b|--bam        --arp cheating\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-d|--hardware   --get mac address\033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-g|--gateway    --get gateway ip address \033[0m\n");  
    printf("\033[0;31m/********************Program Usage***********************/\033[0m\n");  
} 

/**
 * @brief Get parameters from the command line 
 *
 * @param agrc   [in] the count of paramters
 * @param agrv[] [in] parameters array
 *
 * @return 0, if succ
 *        -1, if failed
 */
static int parser_args(int agrc, char *agrv[])
{
    int opt = 0;
    const char *optstr = "hcsb:ra:i:p:t:w:m:e:g::d::n:";
    struct option opts[] = {
        { "help"     , 0, 0, 'h'},
        { "agreement", 1, 0, 'a'},
        { "client"   , 0, 0, 'c'},
        { "server"   , 0, 0, 's'},
        { "ip"       , 1, 0, 'i'},
        { "ether"    , 2, 0, 'e'},
        { "port"     , 1, 0, 'o'},
        { "times"    , 1, 0, 't'},
        { "wait"     , 1, 0, 'w'},
        { "message"  , 1, 0, 'm'},
        { "gateway"  , 2, 0, 'g'},
        { "hardware" , 2, 0, 'd'},
        { "interact" , 0, 0, 'r'},
        { "bam"      , 0, 0, 'b'},
        { "net_mac"  , 0, 0, 'n'},
        {     0      , 0, 0,  0 }
    };

    if (agrc < 2) return -1;

    while ( ( opt = getopt_long( agrc, agrv, optstr, opts, NULL ) ) != -1 ) {
        switch(opt) {
            case 'h':
                print_usage();
                exit(1);
                break;
            case 'b':
                cheat_flag = 1; 
                if (optarg != NULL) strcpy(ip, optarg);
                break;
            case 'c':
                cli_flag = 1; 
                break;
            case 's':
                ser_flag = 1; 
                break;
            case 'q':
                interact_flag = 1; 
                break;
            case 'i':
                if (optarg != NULL) strcpy(ip, optarg);
                if (strlen(ip)) {
                    if (!match_ip(ip)) {
                        printf("\033[0;31mPlease input right ip address!\033[0m\n");
                        exit(1);
                    }
                }
                break;
            case 'e':
                eth_flag = 1;
                if (optarg != NULL) strcpy(ether, optarg);
                break;
            case 'g':
                gateway_flag = 1;
                if (optarg != NULL) strcpy(gateway_ip, optarg);
                break;
            case 'm':
                if (optarg != NULL) strcpy(message, optarg);
                break;
            case 'n':
                netmac_flag = 1;
                if (optarg != NULL) strcpy(ip, optarg);
                break;
            case 'p':
                if (optarg != NULL) port = atoi(optarg);
                if (port < 0 || port > 65535) 
                {
                    printf("port input error\n");
                    exit(1);
                }
                break;
            case 'w':
                if (optarg != NULL)
                    sleep_time = atoi(optarg);
                break;
            case 't':
                if (optarg != NULL)
                    times = atoi(optarg);
                break;
            case 'd':
                mac_flag = 1;
                if (optarg != NULL) strcpy(ether, optarg);
                break;
            case 'a':
                if (!strcasecmp(optarg, "t") || !strcasecmp(optarg, "tcp")) {
                    protocol = PROTO_TCP;
                    strcpy(proto_str, "TCP");
                }
                else if (!strcasecmp(optarg, "u") || !strcasecmp(optarg, "udp")) {
                    protocol = PROTO_UDP;
                    strcpy(proto_str, "UDP");
                }
                else if (!strcasecmp(optarg, "s") || !strcasecmp(optarg, "sctp")) {
                    protocol = PROTO_SCTP;
                    strcpy(proto_str, "SCTP");
                }
                else if (!strcasecmp(optarg, "b") || !strcasecmp(optarg, "broadcast")) {
                    protocol = PROTO_BROADCAST;
                    strcpy(proto_str, "Broadcast");
                }
                else if (!strcasecmp(optarg, "m") || !strcasecmp(optarg, "multicast")) {
                    protocol = PROTO_MULTICAST;
                    strcpy(proto_str, "Multicast");
                }
                else {
                    printf("protocol type error\n");
                    exit(1);
                }
                break;
            case '?':
            default:
                return -1;
        }
    }

    return 0;
}
