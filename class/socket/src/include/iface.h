#ifndef __IFACE_H__
#define __IFACE_H__

#define TAP_DEVICE "/dev/net/tun"
typedef struct iface_t iface_t;
struct iface_t {

};

iface_t *iface_create(char *ifname);
#endif /* __IFACE_H__ */
