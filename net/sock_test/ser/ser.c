#include <sock.h>
void on_recv(struct sock *sck, int cli_fd_i)
{
    char buf[128] = {0};

    recv(sck->cli_fd[cli_fd_i], buf, sizeof(buf), 0);
    printf("On Recv: %s\n", buf);
    //send(sck->cli_fd[cli_fd_i], "hello", sizeof("hello"), 0);

    return;
}

void on_close(struct sock *sck, int cli_fd_i)
{
    printf("On Close.\n");
    return;
}

int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    struct sock ser = {0};

    tcp_server_create(&ser, 5001, on_recv, on_close);
    sleep(180);

    return rt;
}
