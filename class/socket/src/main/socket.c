/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <host.h>
#include <socket.h>
#include <get_args.h>
#include <utils.h>
#include <property.h>
#include <ftp.h>
#include <socket_base.h>
#include <arp.h>
#include <cast.h>

/*********************************************************
 ***************    Macros Declaration    ****************
 *********************************************************/
enum proto_type_t {
    PROTOCOL_TCP = 1,
    PROTOCOL_UCP = (1 << 1)
};

enum ser_flag_t {
    SOCKET_SERVER = 1,
    SOCKET_CLIENT = (1 << 1)
};

/*********************************************************
 **************    Variable Declaration    ***************
 *********************************************************/

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/
/**
 * parser_args
 */
void parser_args(int agrc, char *agrv[]);

/**
 * @brief cast_to_net -- cast message to network 
 *
 * @param broadcast_ip [in] broadcast ip
 * @param multicast_ip [in] multicast ip 
 * @param port         [in] port
 * @param message      [in] message casting
 */
static int cast_to_net(char *broadcast_ip, char *multicast_ip, int port, char *message);

/**
 * @brief start network 
 *
 * @param ser_or_cli_flag   flag of server or client
 * @param net_type          type of network [AF_INET, AF_INET6]
 * @param socket_type       type socket socket [SOCK_DGRAM, SOCK_STREAM and so on]
 * @param socket_protocol   protocol of network [tcp, udp, 0 and so on]
 * @param ip                ip address
 * @param port              port of socket
 * @param times             times of message sending
 * @param message           message of sending
 */
static void start_network(int ser_or_cli_flag, int net_type, int socket_type, int socket_protocol, char *ip, int port, int times, char *message);

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    /**
     * cmd parameter
     */ 
    int  help_flag             = 0;
    int  ipv6_flag             = 0;
    int  ser_flag              = 0;
    int  cli_flag              = 0;
    int  ser_or_cli_flag       = -1;
    int  socket_protocol       = -1;
    int  socket_type           = -1;
    int  net_type              = AF_INET;
    int  times                 = 1;
    char *protocol             = NULL;
    char *message              = NULL;
    char *ip                   = NULL;
    int  port                  = 0;
    int  rt                    = 0;
    int  local_ip_flag         = 0;
    char local_ip[256]         = {0};
    int gateway_ip_flag        = {0};
    char gateway_ip[128]       = {0};
    char *ifname               = NULL;
    char *remote_mac           = NULL;
    char remote_ip[20]         = {0};
    char *broadcast_ip         = NULL;
    char *multicast_ip         = NULL;
    unsigned char local_mac[6] = {0};

    struct options opts[] = {
        {"-h", "--help"      , 0, RET_INT, ADDR_ADDR(help_flag)      },
        {"-6", NULL          , 0, RET_INT, ADDR_ADDR(ipv6_flag)      },
        {"-s", "--server"    , 0, RET_INT, ADDR_ADDR(ser_flag)       },
        {"-c", "--client"    , 0, RET_INT, ADDR_ADDR(cli_flag)       },
        {"-a", "--agreement" , 1, RET_STR, ADDR_ADDR(protocol)       },
        {"-i", "--ip"        , 1, RET_STR, ADDR_ADDR(ip)             },
        {"-p", "--port"      , 1, RET_INT, ADDR_ADDR(port)           },
        {"-t", "--times"     , 1, RET_INT, ADDR_ADDR(times)          },
        {"-m", "--message"   , 1, RET_STR, ADDR_ADDR(message)        },
        {"-l", "--localip"   , 0, RET_INT, ADDR_ADDR(local_ip_flag)  },
        {"-g", "--gatewayip" , 0, RET_INT, ADDR_ADDR(gateway_ip_flag)},
        {NULL, "--mac"       , 1, RET_STR, ADDR_ADDR(ifname)         },
        {"-r", "--remoteip"  , 1, RET_STR, ADDR_ADDR(remote_mac)     },
        {NULL, "--broadcast" , 1, RET_STR, ADDR_ADDR(broadcast_ip)   },
        {NULL, "--multicast" , 1, RET_STR, ADDR_ADDR(multicast_ip)   },
        {NULL, NULL}
    };
    struct usage help_usage[] = {
        {"-s, --server",                 "Create a socket server"},
        {"-c, --client",                 "Create a socket client"},
        {"-a, --agreement [Agreement]",  "Agreement of networking. Agreement can be \"[u udp t tcp]\""},
        {"-i, --ip [ip address]",        "IP address"},
        {"-p, --port [port]",            "Port"},
        {"-t, --times [times]",          "Times of sending message"},
        {"-m, --message [message]",      "Message of sending"},
        {"-l, --localip",                "Local ip address"},
        {"-g, --gatewayip",              "Gateway ip address"},
        {"-r, --remoteip [mac address]", "Get remote ip address."},
        {"-h, --help",                   "Program usage"},
        {"--mac [interface name]",       "Get mac address by interface name."},
        {"--broadcast [broadcast ip]",   "broadcast."},
        {"--multicast [multicast ip]",   "multicast."},
        {NULL,                           NULL}
    };
 
    /**
     * check count of cmdline arguemnts
     */
    set_print_usage_width(60);
    if (agrc <= 1) {
       print_usage(help_usage);
       exit(-1);
    }

    /**
     * parser args
     */
    get_args(agrc, agrv, opts);
    if(help_flag > 0) {
        print_usage(help_usage);
        exit(1);
    }

    /**
     * get mac addree by interface name
     */
    if (ifname) {
        if (get_mac(ifname, local_mac, sizeof(local_mac)) < 0)
            return -1;
        print_mac(local_mac, NULL);
        return 0;
    }

    /**
     * get remote ip address by mac address
     */
    if (remote_mac) {
        if (get_remote_ip_by_mac(remote_mac, remote_ip, sizeof(remote_ip), 0) < 0)
            return -1;
        printf("%s\n", remote_ip);
        return 0;
    }

    /**
     * Get Local IP Address
     */
    if (ipv6_flag > 0) net_type = AF_INET6;
    if (local_ip_flag) {
        if (get_local_ip(net_type, NULL, local_ip, sizeof(local_ip)) < 0) {
            exit(-1);
        }
        printf("%s\n", local_ip);
        exit(0);
    }

    /**
     * get gateway ip address
     */ 
    if (gateway_ip_flag) {
        if (get_gateway(gateway_ip, sizeof(gateway_ip)) < 0) {
            exit(-1);
        }
        printf("%s\n", gateway_ip);
        exit(0);
    }

    /**
     * cast to net 
     */
    if (broadcast_ip || multicast_ip) {
        cast_to_net(broadcast_ip, multicast_ip, port, message);
        return 0;
    }

    /**
     * socket server or client
     */
    if (ser_flag > 0) ser_or_cli_flag = SOCKET_SERVER;
    else if (cli_flag > 0) ser_or_cli_flag = SOCKET_CLIENT;

    /**
     * socket protocol
     */
    if (protocol != NULL) {
        if (!strncmp("u", protocol, sizeof("u")) || !strncmp("udp", protocol, sizeof("udp"))) {
            socket_protocol = IPPROTO_UDP;
            socket_type     = SOCK_DGRAM;
        } else if (!strncmp("t", protocol, sizeof("t")) || !strncmp("tcp", protocol, sizeof("tcp"))) {
            socket_protocol = IPPROTO_TCP;
            socket_type     = SOCK_STREAM;
        } else {
            fprintf(stderr, "Invalid arguemnt\n");
            exit(1);
        }
        
    }

    /**
     * start up socket
     */
    start_network(ser_or_cli_flag, net_type, socket_type, socket_protocol, ip, port, times, message);

    return rt;
}

/**
 * @brief cast_to_net -- cast message to network 
 *
 * @param broadcast_ip [in] broadcast ip
 * @param multicast_ip [in] multicast ip 
 * @param port         [in] port
 * @param message      [in] message casting
 */
static int cast_to_net(char *broadcast_ip, char *multicast_ip, int port, char *message)
{
    cast_t *cast      = NULL;
    int ret           = 0;
    int snd_cnt       = 0;
    char rcv_buf[100] = {0};
    enum cast_type_t {
        BROAD_CAST = 0,
        MULTI_CAST,
        NULL_CAST
    };
    struct {
        char *ip;
        enum cast_type_t type;
    } cast_net[3] = {
        {broadcast_ip, BROAD_CAST},
        {multicast_ip, MULTI_CAST},
        {NULL, NULL_CAST}
    };
    int i  = 0;
    int rt = 0;

    if (port < 1) port = 5001;
    while (i < 3) {
        if (!cast_net[i].ip) goto next;
        if (cast_net[i].type == BROAD_CAST) 
            cast = create_broadcast(broadcast_ip, port);
        else if (cast_net[i].type == MULTI_CAST) 
            cast = create_multicast(multicast_ip, port);
        else break;
        if (!cast) goto next;

        if (message) {
            snd_cnt = strlen(message);
            ret = cast->send(cast, message, snd_cnt);
            if (ret != snd_cnt) {
                if (cast_net[i].type == BROAD_CAST)
                    printf("broadcast send failed\n");
                else
                    printf("multicast send failed\n");
                rt = -1;
            } else {
                if (cast_net[i].type == BROAD_CAST)
                    printf("broadcast send succ\n");
                else
                    printf("multicast send succ\n");
            }
        } else {
            ret = cast->recv(cast, rcv_buf, sizeof(rcv_buf));
            if (ret > 0) {
                if (cast_net[i].type == BROAD_CAST)
                    printf("broadcast recv: %s\n", rcv_buf);
                else
                    printf("multicast recv: %s\n", rcv_buf);
            } else {
                if (cast_net[i].type == BROAD_CAST)
                    printf("broadcast recv failed.\n");
                else printf("multicast recv failed.\n");
                rt = -1;
            }
        }
        cast->destroy(cast);

next:
        i++;
    }

    return rt;
}

/**
 * @brief start network 
 *
 * @param ser_or_cli_flag   flag of server or client
 * @param net_type          type of network [AF_INET, AF_INET6]
 * @param socket_type       type socket socket [SOCK_DGRAM, SOCK_STREAM and so on]
 * @param socket_protocol   protocol of network [tcp, udp, 0 and so on]
 * @param ip                ip address
 * @param port              port of socket
 * @param times             times of message sending
 * @param message           message of sending
 */
void start_network(int ser_or_cli_flag, int net_type, int socket_type, int socket_protocol, char *ip, int port, int times, char *message)
{
    int status = 0;
    socket_t *sck = NULL;
    char buf[512] = {0};

    if (ser_or_cli_flag <= 0) return;
    
    if (socket_type <= 0 || socket_protocol <= 0) {
        fprintf(stderr, "[socket]: please give protocol type when create a server or client socket\n");
        exit(1);
    }
    if (ip == NULL) {
        ip = "%any";
        /*
        if (ser_or_cli_flag == SOCKET_CLIENT) {
            fprintf(stderr, "[socket]: please give ip address when create a server or client socket\n");
            exit(1);
        }
        */
    }
    if (port < 1) {
        port = 5001;
        /*
        fprintf(stderr, "[socket]: please give port number when create a server or client socket\n");
        exit(1);
        */
    }

    sck = create_socket();
    if (sck == NULL) {
        fprintf(stderr, "[socket]: socket_create failed\n");
    }
    switch (ser_or_cli_flag) {
        case SOCKET_SERVER:
            status = sck->listen(sck, net_type, socket_type, socket_protocol, ip, port);
            if (status <= 0) break;
            if (socket_type == SOCK_STREAM) status = sck->accept(sck);
            if (status <= 0) break;

            fprintf(stdout, "----->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            while (sck->recv(sck, buf, sizeof(buf), 0) > 0 ) {
                if (!strncasecmp("==end==", buf, sizeof("==end=="))) {
                    break;
                }
                fprintf(stdout, "[socket recv from %s]: %s\n", sck->get_cli_ip(sck), buf);
            }
            fprintf(stdout, "----->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            break;
        case SOCKET_CLIENT:
            status = sck->connect(sck, net_type, socket_type, socket_protocol, ip, port);
            if (status <= 0) break;
            if (message == NULL) break;

            fprintf(stdout, "----->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            while (times-- > 0) {
                status = sck->send(sck, message, strlen(message));
                if (status > 0)fprintf(stdout, "[socket send to %s]: %s\n", ip, message);
                if (times > 0)sleep(1);
            }
            usleep(100);
            if (socket_type == SOCK_DGRAM) {
                sck->send(sck, "==end==", strlen("==end=="));
            }
            fprintf(stdout, "----->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            break;
    }

    if (sck != NULL) sck->destroy(sck);
}
