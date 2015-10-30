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
 * @brief print_usage 
 */
//static void print_usage();

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int help_flag = 0;
    int ser_flag = 0;
    int cli_flag = 0;
    int ser_or_cli_flag = -1;
    int socket_protocol= -1;
    int socket_type = -1;
    int send_times = 1;
    char *protocol = NULL;
    char *message = NULL;
    char *ip = NULL;
    int port = NULL;
    int rt = 0;
    int status = 0;
    socket_t *sck = NULL;
    char buf[512] = {0};
    struct options opts[] = {
        {"-h", "--help", 0, RET_INT, ADDR_ADDR(help_flag)},
        {"-s", "--server", 0, RET_INT, ADDR_ADDR(ser_flag)},
        {"-c", "--client", 0, RET_INT, ADDR_ADDR(cli_flag)},
        {"-a", "--agreement", 1, RET_STR, ADDR_ADDR(protocol)},
        {"-i", "--ip", 1, RET_STR, ADDR_ADDR(ip)},
        {"-p", "--port", 1, RET_INT, ADDR_ADDR(port)},
        {"-m", "--message", 1, RET_STR, ADDR_ADDR(message)},
    };
    
    get_args(agrc, agrv, opts);
    printf("help_flag: %d\n", help_flag);
    printf("ip: %s\n", ip);
    printf("port: %d\n", port);
    printf("message: %s\n", message);
    printf("ser_flag: %d\n", ser_flag);
    printf("cli_flag: %d\n", cli_flag);
    printf("protocol: %s\n", protocol);

    return 0;
    if (ser_flag > 0) ser_or_cli_flag = SOCKET_SERVER;
    else if (cli_flag > 0) ser_or_cli_flag = SOCKET_CLIENT;
    if (ser_or_cli_flag > 0) {
        if (socket_type <= 0 || socket_protocol <= 0) {
            fprintf(stderr, "[socket]: please give protocol type when create a server or client socket\n");
            exit(1);
        }
        if (ip == NULL) {
            fprintf(stderr, "[socket]: please give ip address when create a server or client socket\n");
            exit(1);
        }
        if (port < 1) {
            fprintf(stderr, "[socket]: please give port number when create a server or client socket\n");
            exit(1);
        }

        sck = socket_create();
        if (sck == NULL) {
            fprintf(stderr, "[socket]: socket_create failed\n");
        }
        switch (ser_or_cli_flag) {
            case SOCKET_SERVER:
                status = sck->listen(sck, AF_INET, socket_type, socket_protocol, ip, port);
                if (status <= 0) break;
                if (socket_type == SOCK_STREAM) status = sck->accept(sck);
                if (status <= 0) break;

                fprintf(stdout, "----->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
                while (sck->receive(sck, buf, sizeof(buf), 0) > 0 ) {
                    if (!strncasecmp("==end==", buf, sizeof("==end=="))) {
                        break;
                    }
                    fprintf(stdout, "[socket receive from %s]: %s\n", sck->get_cli_ip(sck), buf);
                }
                fprintf(stdout, "----->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
                break;
            case SOCKET_CLIENT:
                status = sck->connect(sck, AF_INET, socket_type, socket_protocol, ip, port);
                if (status <= 0) break;
                if (message == NULL) break;
                
                fprintf(stdout, "----->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
                while (send_times-- > 0) {
                    status = sck->send(sck, message, strlen(message));
                    if (status > 0)fprintf(stdout, "[socket send to %s]: %s\n", ip, message);
                    if (send_times > 0)sleep(1);
                }
                if (socket_type == SOCK_DGRAM) {
                    sck->send(sck, "==end==", strlen("==end=="));
                }
                fprintf(stdout, "----->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
                break;
        }

        if (sck != NULL) sck->destroy(sck);
    }

    return rt;
}

/**
 * @brief print_usage 
 */
/*
static void print_usage()
{
    printf("Usage: socket [options] [parameter]\n");
    printf("The Options arg:\n");
    printf("  -s, --server     Create a socket server\n");
    printf("  -c, --client     Create a socket client\n");
    printf("  -a, --agreement  Agreement of networking. Agreement can be \"[u udp t tcp]\"\n");
    printf("  -i, --ip         IP address\n");
    printf("  -p, --port       Port\n");
    printf("  -t, --send_times times of sending message\n");
    printf("  -m, --message    Message of sending\n");
}

static void check_args_num(int agrc, char *agrv[], int curr_num)
{
    if (curr_num >= agrc) {
        fprintf(stderr, "socket: %s option must has a parameter!\n", agrv[curr_num - 1]);
        exit(1);
    }
}
*/

/**
 * parser_args
 */
/*
void parser_args(int agrc, char *agrv[]){
    int i = 0;
    
    if (agrc < 1) {
        return;
    }

    for (i = 0; i < agrc; i++) {
        if (!strncmp("-h", agrv[i], sizeof("-h")) || !strncmp("--help", agrv[i], sizeof("--help"))) {
            print_usage();
        } else if (!strncmp("-s", agrv[i], sizeof("-s")) || !strncmp("--server", agrv[i], sizeof("--server"))) {
            ser_or_cli_flag = SOCKET_SERVER;
        } else if (!strncmp("-c", agrv[i], sizeof("-c")) || !strncmp("--client", agrv[i], sizeof("--client"))) {
            ser_or_cli_flag = SOCKET_CLIENT;
        } else if (!strncmp("-a", agrv[i], sizeof("-a")) || !strncmp("--agreement", agrv[i], sizeof("--agreement"))) {
            check_args_num(agrc, agrv, ++i);
            if (!strncmp("u", agrv[i], sizeof("u")) || !strncmp("udp", agrv[i], sizeof("udp"))) {
                socket_protocol = IPPROTO_UDP;
                socket_type = SOCK_DGRAM;
            } else if (!strncmp("t", agrv[i], sizeof("t")) || !strncmp("tcp", agrv[i], sizeof("tcp"))) {
                socket_protocol = IPPROTO_TCP;
                socket_type = SOCK_STREAM;
            } else {
                fprintf(stderr, "Invalid arguemnt\n");
                exit(1);
            }
        } else if (!strncmp("-i", agrv[i], sizeof("-i")) || !strncmp("--ip", agrv[i], sizeof("--ip"))) {
            check_args_num(agrc, agrv, ++i);
            ip = agrv[i];
        } else if (!strncmp("-p", agrv[i], sizeof("-p")) || !strncmp("--port", agrv[i], sizeof("--port"))) {
            check_args_num(agrc, agrv, ++i);
            port = agrv[i];
        } else if (!strncmp("-t", agrv[i], sizeof("-t")) || !strncmp("--send_times", agrv[i], sizeof("--send_times"))) {
            check_args_num(agrc, agrv, ++i);
            send_times = atoi(agrv[i]) > 0 ? atoi(agrv[i]) : send_times;
        } else if (!strncmp("-m", agrv[i], sizeof("-m")) || !strncmp("--message", agrv[i], sizeof("--message"))) {
            check_args_num(agrc, agrv, ++i);
            send_message = agrv[i];
        }
    }

}
*/
