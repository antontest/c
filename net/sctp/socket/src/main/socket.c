/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <socket/socket_app.h>

/*********************************************************
 *****************    Global Variable    *****************
 *********************************************************/
static char ip[256] = "127.0.0.1";
static unsigned short protocol = 0;
static unsigned short port = 5001;
static unsigned int sleep_time = -1;
static unsigned int times = 0;
static char message[1024] = {0};
static int ser_flag = 0;
static int cli_flag = 0;
static int interact_flag = 0;

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
int tcp_ser(const char *ip, const unsigned short port);
int tcp_cli(const char *ip, const unsigned short port);

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
        print_usage();
        rt = -1;
        goto error;
    };

    printf("\033[1;31mConfiguration: \n");
    printf("\033[1;35m  protocol    : \033[1;32m%d\n", protocol);
    printf("\033[1;35m  ip          : \033[1;32m%s\n", ip);
    printf("\033[1;35m  port        : \033[1;32m%d\n", port);
    printf("\033[1;35m  ser         : \033[1;32m%d\n", ser_flag);
    printf("\033[1;35m  cli         : \033[1;32m%d\n", cli_flag);
    printf("\033[1;35m  sleep time  : \033[1;32m%d\n", sleep_time);
    printf("\033[1;35m  times       : \033[1;32m%d\n", times);
    printf("\033[0m\n");

    switch (protocol)
    {
        case PROTO_TCP:
            if (ser_flag) tcp_ser(ip, port);
            else if (cli_flag) tcp_cli(ip, port);
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
    printf("accepted from %s.\n", \
            inet_ntoa(((struct socket_impl *)arg)->addr.in_addr.sin_addr));
}

void on_connect(int fd, void *arg)
{
    printf("connected %s succ.\n", \
            inet_ntoa(((struct socket_impl *)arg)->addr.in_addr.sin_addr));
}

void on_recv(int fd, void *arg)
{
    char buf[128] = {0};
    
    socket_recv(fd, buf, sizeof(buf));
    printf("recv from %s: %s\n", 
            inet_ntoa(((struct socket_impl *)arg)->addr.in_addr.sin_addr), 
            buf);
}

void on_close(int fd, void *arg)
{
    if (cli_flag) printf("server %s closed.\n", \
            inet_ntoa(((struct socket_impl *)arg)->addr.in_addr.sin_addr));
    else printf("client %s closed.\n", \
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
int tcp_ser(const char *ip, const unsigned short port)
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
int tcp_cli(const char *ip, const unsigned short port)
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
        socket_send(sck.fd, message, strlen(message));

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

/**
 * @brief print usage of the pragram 
 */
static void print_usage() 
{
    printf("\033[0;31m/********************Program Usage***********************/\033[0m\n");  
    printf("\033[0;31mFile  : \033[0;32m%s\033[0m\n", __FILE__);  
    printf("\033[0;31mVers  : \033[0;32m1.0.0\033[0m\n");
    printf("\033[0;31mTime  : \033[0;32m2015.06.04\033[0m\n");
    printf("\033[0;31mBrief : \033[0m\n");  
    printf("\033[0;31mUsage : \033[0;32m-f <file_name> -o <output_name>\033[0m\n");  
    printf("\033[0;31mParam : \033[0m\n");
    printf("\033[0;31m        \033[0;32m-f --file       the CDR file to be decoded\033[0m\n");  
    printf("\033[0;31m        \033[0;32m-o --output     the output file in plain text format\033[0m\n");  
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
    const char *optstr = "hcsqa:i:p:t:w:m:";
    struct option opts[] = {
        { "help"     , no_argument      , 0, 'h'},
        { "agreement", required_argument, 0, 'a'},
        { "client"   , no_argument      , 0, 'c'},
        { "server"   , no_argument      , 0, 's'},
        { "ip"       , required_argument, 0, 'i'},
        { "port"     , required_argument, 0, 'o'},
        { "times"    , required_argument, 0, 't'},
        { "wait"     , required_argument, 0, 'w'},
        { "message"  , required_argument, 0, 'm'},
        { "interact" , no_argument      , 0, 'q'},
        {     0    ,       0            , 0,  0 }
    };

    if (agrc < 2) return -1;

    while ( ( opt = getopt_long( agrc, agrv, optstr, opts, NULL ) ) != -1 ) {
        switch(opt) {
            case 'h':
                return -1;
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
                break;
            case 'm':
                if (optarg != NULL) strcpy(message, optarg);
                break;
            case 'p':
                if (optarg != NULL) port = atoi(optarg);
                if (port < 0) 
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
            case 'a':
                if (!strcasecmp(optarg, "t") || !strcasecmp(optarg, "tcp"))
                    protocol = PROTO_TCP;
                else if (!strcasecmp(optarg, "u") || !strcasecmp(optarg, "udp"))
                    protocol = PROTO_UDP;
                else if (!strcasecmp(optarg, "s") || !strcasecmp(optarg, "sctp"))
                    protocol = PROTO_SCTP;
                else if (!strcasecmp(optarg, "b") || !strcasecmp(optarg, "broadcast"))
                    protocol = PROTO_BROADCAST;
                else if (!strcasecmp(optarg, "m") || !strcasecmp(optarg, "multicast"))
                    protocol = PROTO_MULTICAST;
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
