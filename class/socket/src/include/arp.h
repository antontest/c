#ifndef __ARP_H__
#define __ARP_H__

#define ARP_OP_REQUEST htons(ARPOP_REQUEST)
#define ARP_OP_REPLY   htons(ARPOP_REPLY)
typedef enum arp_type_t arp_type_t;
enum arp_type_t {
    ARP_REQUEST = 1,
    ARP_REPLY
};

typedef struct arp_t arp_t;
struct arp_t {
    /**
     * @brief open arp socket
     */
    int (*open) (arp_t *this, char *ifname);

    /**
     * @brief send arp message
     */
    int (*send) (arp_t *this, arp_type_t type, char *dst_mac, char *src_mac, char *dst_ip, char *src_ip);

    /**
     * @brief recv arp message
     */
    int (*recv) (arp_t *this, arp_type_t type, void *buf, int size, int timeout_ms);

    /**
     * @brief socket close
     */
    int (*close) (arp_t *this);

    /**
     * @brief destroy instance and free memory 
     */
    void (*destroy) (arp_t *this);

    /**
     * @brief get socket fd
     */
    int (*get_fd) (arp_t *this);
};

/**
 * @brief create arp instance
 */
arp_t *arp_create();

/**
 * @brief get remote ip by mac address 
 *
 * @param mac        [in]  mac address
 * @param ip         [out] remote ip address
 * @param size       [in]  size of ip buffer
 * @param timeout_ms [in]  timeout
 *
 * @return 0, if succ; -1, if timeout or failed
 */
int get_remote_ip_by_mac(char *mac, char *ip, int size, int timeout_ms);

#endif /* __ARP_H__ */
