#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <cast.h>

int main(int argc, char **argv)
{
    int ret = 0;
    char buf[128] = {0};

    //cast_t *cast = create_broadcast("255.255.255.255", 5001);
    cast_t *cast = create_multicast("225.0.0.88", 5001);

    if (argc < 2) {
        ret = cast->send(cast, "hi", sizeof("hi"));
        printf("ret: %d\n", ret);
        perror("send");
        sleep(1);
        ret = cast->send(cast, "hi", 3);
        printf("ret: %d\n", ret);
        perror("send");
        sleep(1);
    } else {
        ret = cast->recv(cast, buf, sizeof(buf));
        if (ret > 0) {
            printf("recv: %s\n", buf);
        }
        perror("recv");
        ret = cast->recv(cast, buf, sizeof(buf));
        if (ret > 0) {
            printf("recv: %s\n", buf);
        }
        perror("recv");
    }
    cast->destroy(cast);

    return 0;
}
