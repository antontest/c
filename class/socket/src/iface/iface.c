#include <iface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <fcntl.h>
#include <net/if.h>
#include <utils/utils.h>

typedef struct private_iface_t private_iface_t;
struct private_iface_t {
    /**
     * @brief public interface
     */
    iface_t public;

    /**
     * @brief device name at host (tap0)
     */
    char *hostif;

    /**
     * @brief device name in guest (eth0)
     */
    char *guestif;
};
#define iface_hostif this->hostif 
#define iface_guestif this->guestif

METHOD(iface_t, destroy_, void, private_iface_t *this)
{
    if (iface_hostif) free(iface_hostif);
    if (iface_guestif) free(iface_guestif);
    free(this);
}

static char *create_tap(private_iface_t *this)
{
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

    return strdup(ifr.ifr_name);
}

iface_t *iface_create(char *ifname)
{
    private_iface_t *this;

    INIT(this, 
        .public = {
        },
        .hostif  = NULL,
        .guestif = strdup(ifname),
    );

    this->hostif = create_tap(this);

    return &this->public;
}
