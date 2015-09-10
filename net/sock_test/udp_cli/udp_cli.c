/*
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main()
{
    int fd[2] = {-1};
    struct sockaddr_in addr = {0};
    int len = sizeof(struct sockaddr_in);
    char buf[128] = {0};

    fd[0] = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd[0] < 0) perror("socket");
    fd[1] = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd[1] < 0) perror("socket");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5001);

    //if (bind(fd, (struct sockaddr *)&addr, (socklen_t)len) < 0)
    //    perror("bind");

    if (connect(fd[0], (struct sockaddr *)&addr, (socklen_t)len) < 0)
        perror("connect\n");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5002);
    if (bind(fd[1], (struct sockaddr *)&addr, (socklen_t)len) < 0)
        perror("bind");

    len = 0;
    int value = 0;
    while (1)
    {
        sprintf(buf, "%d", value);
        if ((len = send(fd[0], buf, strlen(buf), 0)) > 0)
            printf("cli send count: %d\n", len);
        value++;

        //sleep(1);
        if ((len = recv(fd[1], buf, 128, 0)) > 0)
            printf("cle recv : %s\n", buf);
        sleep(1);
    }

    if ((len = send(fd, "hello", 6, 0)) > 0)
        printf("send count: %d\n", len);
    while (recv(fd, buf, 128, 0) > 0)
    {
        printf("recv: %s\n", buf);
    }

    return 0;
}
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main()
{
    int fd = 0;
    struct sockaddr_in addr = {0};
    int len = sizeof(addr);
    char buf[128] = {0};

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) perror("socket");

    bzero(&addr, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(5001);

    //if (bind(fd, (struct sockaddr *)&addr, (socklen_t)len) < 0)
    //    perror("bind");

    //if (connect(fd[0], (struct sockaddr *)&addr, (socklen_t)len) < 0)
    //    perror("connect\n");

    len = 0;
    int value = 0;
    int rt = 0;
    len = sizeof(addr);
    while (1)
    {
        sprintf(buf, "%d", value);
        if ((rt = sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&addr, \
                        len)) > 0)
            printf("cli send count: %d\n", rt);
        else if (rt == -1)perror("cli sendto");
        value++;

        //sleep(1);
        if ((rt = recvfrom(fd, buf, 128, 0, (struct sockaddr *)&addr, \
                        (socklen_t *)&len)) > 0)
            printf("cle recv : %s\n", buf);
        sleep(1);
    }

    return 0;
}

/************************************************************************* 
 > File Name: client.c 
 > Author: SongLee 
 ************************************************************************/

/*
#include<sys/types.h> 
#include<sys/socket.h> 
#include<unistd.h> 
#include<netinet/in.h> 
#include<arpa/inet.h> 
#include<stdio.h> 
#include<stdlib.h> 
#include<errno.h> 
#include<netdb.h> 
#include<stdarg.h> 
#include<string.h> 
  
#define SERVER_PORT 5001 
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
  
int main() 
{ 

 struct sockaddr_in server_addr = {0}; 
 //bzero(&server_addr, sizeof(server_addr)); 
 server_addr.sin_family = AF_INET; 
 server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
 server_addr.sin_port = htons(SERVER_PORT); 
  
 int client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); 
 if(client_socket_fd < 0) 
 { 
  perror("Create Socket Failed:"); 
  exit(1); 
 }
	
  
 char buffer[BUFFER_SIZE] = "hello"; 
  
 if(sendto(client_socket_fd, buffer, strlen(buffer),0,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0) 
 { 
  perror("Send File Name Failed:"); 
  exit(1); 
 } 
  
 close(client_socket_fd); 
 return 0; 
} 

*/
