/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <host.h>
#include <socket.h>
#include <utils/get_args.h>
#include <utils/utils.h>
#include <property.h>
#include <ftp.h>

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

void start_network(int ser_or_cli_flag, int net_type, int socket_type, int socket_protocol, char *ip, int port, int times, char *message);

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    /**
     * cmd parameter
     */ 
    int  help_flag       = 0;
    int  ipv6_flag       = 0;
    int  ser_flag        = 0;
    int  cli_flag        = 0;
    int  ser_or_cli_flag = -1;
    int  socket_protocol = -1;
    int  socket_type     = -1;
    int  net_type        = AF_INET;
    int  times           = 1;
    char *protocol       = NULL;
    char *message        = NULL;
    char *ip             = NULL;
    int  port            = 0;
    int  rt              = 0;

    struct options opts[] = {
        {"-h", "--help"      , 0, RET_INT, ADDR_ADDR(help_flag) },
        {"-6", NULL          , 0, RET_INT, ADDR_ADDR(ipv6_flag) },
        {"-s", "--server"    , 0, RET_INT, ADDR_ADDR(ser_flag)  },
        {"-c", "--client"    , 0, RET_INT, ADDR_ADDR(cli_flag)  },
        {"-a", "--agreement" , 1, RET_STR, ADDR_ADDR(protocol)  },
        {"-i", "--ip"        , 1, RET_STR, ADDR_ADDR(ip)        },
        {"-p", "--port"      , 1, RET_INT, ADDR_ADDR(port)      },
        {"-t", "--times"     , 1, RET_INT, ADDR_ADDR(times)     },
        {"-m", "--message"   , 1, RET_STR, ADDR_ADDR(message)   },
    };
    struct usage help_usage[] = {
        {"-s, --server"    , "Create a socket server"},
        {"-c, --client"    , "Create a socket client"},
        {"-a, --agreement" , "Agreement of networking. Agreement can be \"[u udp t tcp]\""},
        {"-i, --ip"        , "IP address"},
        {"-p, --port"      , "Port"},
        {"-t, --times"     , "Times of sending message"},
        {"-m, --message"   , "Message of sending"},
        {"-h, --help"      , "Program usage"},
        {NULL              , NULL}
    };
 
    get_args(agrc, agrv, opts);
    if(help_flag > 0) {
        print_usage(help_usage);
        exit(1);
    }

    if (ipv6_flag > 0) net_type = AF_INET6;
    if (ser_flag > 0) ser_or_cli_flag = SOCKET_SERVER;
    else if (cli_flag > 0) ser_or_cli_flag = SOCKET_CLIENT;
    if (protocol != NULL) {
        if (!strncmp("u", protocol, sizeof("u")) || !strncmp("udp", protocol, sizeof("udp"))) {
            socket_protocol = IPPROTO_UDP;
            socket_type = SOCK_DGRAM;
        } else if (!strncmp("t", protocol, sizeof("t")) || !strncmp("tcp", protocol, sizeof("tcp"))) {
            socket_protocol = IPPROTO_TCP;
            socket_type = SOCK_STREAM;
        } else {
            fprintf(stderr, "Invalid arguemnt\n");
            exit(1);
        }
        
    }

    start_network(ser_or_cli_flag, net_type, socket_type, socket_protocol, ip, port, times, message);

    return rt;
}

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
            if (socket_type == SOCK_DGRAM) {
                sck->send(sck, "==end==", strlen("==end=="));
            }
            fprintf(stdout, "----->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            break;
    }

    if (sck != NULL) sck->destroy(sck);
}
