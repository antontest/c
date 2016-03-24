#ifndef __SOCKET_PROPERTY_H__
#define __SOCKET_PROPERTY_H__
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

typedef struct property_t property_t;
struct property_t {
    /**
     * @brief get local machine's IP address
     *
     * @param family AF_INET, AF_INET6 or NULL
     * @param ifname interface name
     * @param ip     ip address buffer
     * @param size   size of buffer
     * @return       0, if succ; -1, if failed
     */
    char *(*get_local_ip) (property_t *this, int family, const char *ifname, char *ip, int size); 

    /**
     * @brief get can_read_bytes in recv buffer 
     */
    int (*get_can_read_bytes) (property_t *this);

    /**
     * @brief  get size of socket recv buffer 
     * @return recv buffer size, if succ; -1, if failed.
     */
    int (*get_recv_buf_size) (property_t *this);

    /**
     * @brief  get size of socket send buffer 
     * @return recv buffer size, if succ; -1, if failed.
     */
    int (*get_send_buf_size) (property_t *this);

    /**
     * @brief get socket timeout of recving
     * @return send timeout, if succ; -1, if failed
     */
    int (*get_recv_timeout) (property_t *this);

    /**
     * @brief get socket timeout of sending 
     * @return send timeout, if succ; -1, if failed
     */
    int (*get_send_timeout) (property_t *this);

    /**
     * @brief get socket protocol 
     * @return protocol type, if succ; -1, if failed
     */
    int (*get_protocol) (property_t *this);

    /**
     * @brief get socket type 
     * @return socket type, if succ; -1, if failed
     */
    int (*get_family) (property_t *this);

    /**
     * @brief get_socket_error 
     * @return socket error, if succ; -1, if failed;
     */
    int (*get_error) (property_t *this);

    /**
     * @brief get_interface_state 
     * @param ifname  [in] interface name
     * @return 1, if up; 0, if down
     */
    int (*get_interface_state) (property_t *this, const char *ifname);
    
    /**
     * @brief set size of socket recv buffer 
     * @param buf_size  [in] buffer size
     * @return 0, if succ; -1, if failed.
     */
    int (*set_recv_buf_size) (property_t *this, int size);

    /**
     * @brief set size of socket send buffer 
     * @param buf_size  [in] buffer size
     * @return 0, if succ; -1, if failed.
     */
    int (*set_send_buf_size) (property_t *this, int size);

    /**
     * @brief set socket timeout of recving
     * @param tm_ms  [in] timeout
     * @return 0, if succ; -1, if failed
     */
    int (*set_recv_timeout) (property_t *this, int tm_ms);

    /**
     * @brief set socket timeout of sending 
     * @param tm_ms  [in] timeout
     * @return 0, if succ; -1, if failed
     */
    int (*set_send_timeout) (property_t *this, int tm_ms);

    /**
     * @brief  set socket unblock
     * @return 0, if succ; -1, if failed
     */
    int (*make_nonblock) (property_t *this);

    /**
     * @brief  set socket block
     * @return 0, if succ; -1, if failed
     */
    int (*make_block) (property_t *this);

    /**
     * @brief make listen socket reuseable
     */
    int (*make_reuseable) (property_t *this, int onoff);

    /**
     * @brief make socket child can't exec 
     */
    int (*make_closenexec) (property_t *this);

    /**
     * @brief make socket keep alive 
     * @return 0, if succ; -1, if failed
     */
    int (*make_keepalive) (property_t *this);

    /**
     * @brief set socket close action 
     * @param onoff  [in] swith of close action
     * @param tm_s   [in] time
     * @return 0, if succ; -1, if failed
     */
    int (*make_close_action) (property_t *this, int onoff, int tm);

    /**
     * @brief set socket broadcast 
     * @param on [in] switch
     * @return 0, if succ; -1, if failed
     */
    int (*make_broadcast) (property_t *this, int onoff);

    /**
     * @brief make network card hybrid mode
     * @param ifname [in] interface name like eth0,eth2 and so on
     * @param on     [in] hybrid mode swith
     * @return 0, if succ; -1, if failed. 
     */
    int (*make_promisc) (property_t *this, const char *ifname, int onff);

    /**
     * @brief set socket multicast loop 
     * @param on [in] switch
     * @return 0, if succ; -1, if failed
     */
    int (*make_multicast_loop) (property_t *this, int onoff);

    /**
     * @brief set socket multicast ttl 
     * @param ttl [in] ttl
     * @return 0, if succ; -1, if failed
     */
    int (*make_multicast_ttl) (property_t *this, int ttl);

    /**
     * @brief add socket to multicast member ship
     * @param mrq [in] struct of multicast memver ship
     * @return  0, if succ; -1, if failed.
     */
    int (*add_to_membership) (property_t *this, struct ip_mreq *mrq);

    /**
     * @brief drop socket from multicast member ship
     * @param mrq [in] struct of multicast memver ship
     * @return  0, if succ; -1, if failed.
     */
    int (*drop_from_membership) (property_t *this, struct ip_mreq *mrq);

    /**
     * @brief set socket descriptor 
     */
    void (*set_fd) (property_t *this, int fd);

    /**
     * @brief destroy instance and free memory 
     */
    void (*destroy) (property_t *this);
};

/**
 * @brief create property instance 
 */
property_t *create_property(int fd);

/*************************************************************
 *****  Function Declaration Of Socket Property Settings  *****
 **************************************************************/
    /**
     * @brief get interface name 
     * @param ifname [out] interface name
     * @return ifname, if succ; NULL, if failed.
     */
    char *get_ifname(char *ifname, int size);

    /**
     * @brief get local machine's ip address.
     * 
     * @param ip[] [out] local ip address.
     *
     * @return 0, if succ; -1, if failed.
     */
    char *get_local_ip(int family, const char *ifname, char *ip, int size);

    /**
     * @brief get hardware address by interface name
     *
     * @param ifname [in]  interface name
     * @param mac    [out] hardware addree
     *
     * @return 0, if succ; -1, if failed
     */
    unsigned char *get_mac(const char *ifname, unsigned char *mac, int size);

    /**
     * @brief detect little endian or big endian
     *
     * @return 1, if big endian; 0, if little endian; -1, if unkown.
     */
    int is_big_endian();

    /**
     * @brief get_eth_rate
     *
     * @param ifname [in] interface name
     *
     * @return interface rate, if succ; -1, if failed
     */
    int get_eth_speed(const char *ifname);

    /**
     * @brief get_gateway -- get ipaddress of gateway
     *
     * @param gateway [out] ip address of gateway
     * @param ifname  [out] interface name
     *
     * @return 0, if succ; -1; if failed
     */
    char *get_gateway(char *gateway, int size);

    /**
     * @brief xioctl 
     *
     * @param fd      [in] socket fd
     * @param request [in] ioctl command
     * @param argp    [out] value or parameters
     * @param fmt     [in] error info
     * @param ...
     *
     * @return 0, if succ; -1, if failed 
     */
    int xioctl(int fd, unsigned int request, void *argp, const char *fmt, ...);


    /**
     * @brief get_net_mac 
     *
     * @param ip     [in]  target ip address
     * @param mac[6] [out] target mac address
     *
     * @return 0, if uscc; -1, if failed
     */
    unsigned char *get_net_mac(char *dstip, int timeout);


    /*************************************************************
     *****  Function Declaration Of Socket String Dealint  ********
     **************************************************************/
    /**
     * @brief ip2arr -- The IP address of the string is converted to an array
     *
     * @param ip     [in]  string of ip address
     * @param arr[4] [out] array of ip address
     */
    void ip2arr(const char *ip, unsigned char arr[4]);

    /**
     * @brief arr2ip 
     *
     * @param ip_arri [in] array of ip address
     * @param ip     [out] string of ip address
     */
    void arr2ip(unsigned char ip_arr[], char *ip);

    /**
     * @brief arr2mac
     *
     * @param mac_arr [in] array of mac address
     * @param mac[]  [out] string of mac address 
     */
    void mac2arr(const char *mac, unsigned char mac_arr[6]);

    /**
     * @brief arr2mac
     *
     * @param mac_arr [in] array of mac address
     * @param mac[]  [out] string of mac address 
     */
    void arr2mac(const unsigned char *mac_arr, char mac[]);

    /**
     * @brief printf_mac 
     *
     * @param mac  [in] mac
     * @param info [in] info
     */
    void print_mac(const unsigned char *mac, const char *info);

    /**
     * @brief printf_mac 
     *
     * @param mac  [in] mac
     * @param info [in] info
     */
    void print_ipv4(const unsigned char *ip, const char *info);

#endif

