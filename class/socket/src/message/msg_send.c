#include <message.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct mymsg {
    msg_t msg;
    int  id;
};
int main(int argc, char **argv)
{
/*     msg_t msg = {0}; */
/*     msg.src_mod = MOD_ID_UNKNOW; */
/*     msg.dst_mod = MOD_ID_CTL; */
/*     msg.msg_len = 4; */
/*     struct mymsg my; */
/*     memcpy(&my, &msg, sizeof(my)); */
/*     my.id = 100; */
/*     msg_hdl->send(msg_hdl, (msg_t *)&my, sizeof(my)); */
/*     msg_hdl->send(msg_hdl, (void *)&my, sizeof(my)); */
    
    int data = 100;
    msg_t *msg = create_new_msg(MOD_ID_UNKNOW, MOD_ID_CTL, 0, &data, sizeof(data));
    msg_handler_t *msg_hdl = create_msg_handler();
    msg_hdl->send(msg_hdl, msg);
    msg_hdl->destroy(msg_hdl);
    free(msg);

    return 0;
}
