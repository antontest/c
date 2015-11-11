/**
 * usual head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*********************************************************
 **************    Function Declaration    ***************
 *********************************************************/

/*********************************************************
 ******************    Main Function    ******************
 *********************************************************/
int main(int agrc, char *agrv[])
{
    int rt = 0; /* return value of function main */
    struct sockaddr_in addr;
    int fd = 0;
    char buf[2048];

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 1) return -1;
    addr.sin_port = htons(21);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("172.21.34.18");
    if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) return -1;
    printf("connect succ\n");
    recv(fd, buf, sizeof(buf), 0);
    printf("ftp: %s", buf);

    send(fd, "USER antonio\r\n", sizeof("USER antonio\r\n"), 0);
    recv(fd, buf, sizeof(buf), 0);
    printf("user: %s", buf);
    send(fd, "PASS 12345\r\n", sizeof("PASS 12345\r\n"), 0);
    recv(fd, buf, sizeof(buf), 0);
    printf("pass: %s", buf);

    send(fd, "PASV\r\n", sizeof("PASV\r\n"), 0);
    recv(fd, buf, sizeof(buf), 0);
    printf("pasv: %s", buf);

    char *p = NULL;
    p = strtok(buf, "(");
    int p1, p2;
    p = strtok(NULL, "(");
    sscanf(p, "%*d,%*d,%*d,%*d,%d,%d", &p1, &p2);
    int data_port = p1 * 256 + p2;
    printf("data_port: %d\n", data_port);

    addr.sin_port = htons(data_port);
    int data_fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(data_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)); 

    send(fd, "LIST\r\n", sizeof("LIST\r\n"), 0);
    recv(data_fd, buf, sizeof(buf), 0);
    printf("list: %s", buf);



    return rt;
}
