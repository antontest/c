#include <socket_icmp.h>

/**
 * @brief check_sum 
 *
 * @param addr
 * @param len
 *
 * @return 
 */
unsigned short check_sum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short ck_sum = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft ==1)
    {
        *(unsigned char *)(&ck_sum) = *(unsigned char *)w;
        sum += sum;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    ck_sum = ~sum;

    return ck_sum;
}

int myping(const char *dst)
{
    int fd = -1;
    struct hostent *host = NULL;
    struct sockaddr_in addr = {0};

    if (dst == NULL) return -1;

    if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        perror("socket()");
        exit(0);
    }

    setuid(getuid());
    addr.sin_family = AF_INET;
    if (inet_addr(dst) == INADDR_NONE)
    {
        if ((host = gethostbyname(dst)) == NULL)
        {
            perror("gethostbyname()");
            exit(1);
        }
        memcpy(&addr.sin_addr, host->h_addr, host->h_length);
    }
    else addr.sin_addr.s_addr = inet_addr(dst);
    
    return 0;
}
