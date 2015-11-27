#include <message.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct mymsg {
    msg_t msg;
    int   id;
};

static void msg_handler(msg_t *msg)
{
    printf("id %d, mod %d: recv message\n", ((struct mymsg *)msg)->id, msg->dst_mod);
}

int main(int argc, char **argv)
{
    mod_cfg_t cfg;
    cfg.id = MOD_ID_CTL;
    cfg.name = "ctl";
    cfg.handler = msg_handler;
    msg_mod_t *mod = create_msg_mod();

    if (mod->act(mod, &cfg)) return -1;

    sleep(20);
    mod->destroy(mod); 
    return 0;
}
