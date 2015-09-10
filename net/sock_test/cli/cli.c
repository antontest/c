#include <sock.h>

void on_connect(int fd, void *arg)
{
    printf("on connect\n");

    return;
}

void on_recv(int fd, void *arg)
{
    //printf("on recv\n");

    return;
}

void on_close(int fd, void *arg)
{
    printf("on close\n");

    return;
}

int main(int agrc, char *agrv[])
{
    struct socket cli = {0};
    //char ip[20] = "/tmp/unix.sock";
    char ip[20] = {0};
    //char buf[20] = {0};
    int value = 111;
    //sock_data_t data = {0};
    //data.type = INT;
    //data.size = sizeof(int);
    //data.value.i = 100;

    get_local_ip(ip);
    //udp_broadcast_send("172.21.22.255", 5001, 10, ip);
    udp_multicast_send("224.0.0.1", 5001, 10, ip);

    return 0;
    get_local_ip(ip);
    //socket_time_connect(cli.fd, &cli.addr, 2000);
    //printf("connect\n");
    socket_event_add(&cli.evl, SOCKET_ON_RECV, on_recv, "recv");
    socket_event_add(&cli.evl, SOCKET_ON_CONNECT, on_connect, "connect");
    socket_event_add(&cli.evl, SOCKET_ON_CLOSE, on_close, "close");
    client_connect(&cli, AF_INET, SOCK_STREAM, ip, 5001);
    sleep(1);

    //if (socket_data_sendto(cli.fd, INT, &value, sizeof(int), ip, 5001) > 0)
    //    printf("send succ\n");
    //if (socket_addr_data_sendto(cli.fd, INT, &value, sizeof(int), &cli.addr) > 0)
    //    printf("send succ\n");
    //else perror("sendto");
    
    //socket_addr_data_recvfrom(cli.fd, NULL, &value, sizeof(int), &cli.addr);
    //printf("value = %d\n", value);
    //return 0;

    socket_data_send(cli.fd, INT, &value, sizeof(int));
    //socket_data_time_recv(cli.fd, NULL, &value, sizeof(int), 2000);
    socket_data_recv(cli.fd, NULL, &value, sizeof(int));
    //socket_data_recv(cli.fd, NULL, &value, sizeof(int));
    printf("value = %d\n", value);
    //socket_send(cli.fd, &data, SOCK_DATA_SIZE);
    //while (1) usleep(100);
    sleep(1);
    //socket_data_type type = -1;
    //udp_socket_data_recv(cli.fd, &type, &value, sizeof(value));
    //socket_recv(cli.fd, buf, sizeof(buf));
    //recv(cli.fd, buf, sizeof(buf), 0);
    //printf("recv: %d\n", value);
    //socket_close(cli.fd);

    return 0;
}
