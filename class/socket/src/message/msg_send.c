#include <message.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    msg_t msg = {0};
    msg.src_mod = MOD_ID_UNKNOW;
    msg.dst_mod = MOD_ID_CTL;
    msg.msg_len = 0;
    
    send_msg(&msg);

    return 0;
}
