#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <packet.h>
#include <utils/utils.h>

typedef enum socket_state_t socket_state_t;
enum socket_state_t {
    SOCKET_CONNECT_ERROR = 0,
    SOCKET_SEND_ERROR,
    SOCKET_RECEIVE_ERROR, 
    SOCKET_CLOSED,
    SOCKET_STARTING,
    SOCKET_CONNECTING,
    SOCKET_CONNECTED,
    SOCKET_SENDING,
    SOCKET_SENDED,
    SOCKET_RECEIVING,
    SOCKET_RECEIVED,
};

typedef struct socket_t socket_t;
typedef enum socket_family_t socket_family_t;

/**
 * Constructor prototype for sockets.
 */
typedef socket_t *(*socket_constructor_t)();

/**
 * Address families supported by socket implementations.
 */
enum socket_family_t {
    /**
     * No address families supported
     */
    SOCKET_FAMILY_NONE = 0,

    /**
     * IPv4
     */
    SOCKET_FAMILY_IPV4 = (1 << 0),

    /**
     * IPv6
     */
    SOCKET_FAMILY_IPV6 = (1 << 1),

    /**
     * Both address families supported
     */
    SOCKET_FAMILY_BOTH = (1 << 2) - 1,
};


/**
 * Socket interface definition.
 */
struct socket_t {
    /**
     * @brief Create a socket
     *
     * @param family        AF_INET, AF_INET6 and so on 
     * @param type          SOCK_STREAM, SOCK_DGRAM and so on
     * @param prototype     IPPROTO_UDP, IPPROTO_TCP and so on
     * @return
     *						- SUCCESS when packet successfully created
     *						- FAILED when unable to create
     */
    status_t (*socket)(socket_t *this, int family, int type, int prototype);

    /**
     * @brief listen on a socket for receiving and sending message
     *
     * @param family        AF_INET, AF_INET6 and so on 
     * @param type          SOCK_STREAM, SOCK_DGRAM and so on
     * @param prototype     IPPROTO_UDP, IPPROTO_TCP and so on
     * @param ip            ip address listening on
     * @param port          port of listening
     * @return
     *						- SUCCESS when packet successfully created
     *						- FAILED when unable to create
     */
    int (*listen) (socket_t *this, int family, int type, int prototype, char *ip, unsigned short port);

    /**
     * @brief accept client
     */
    int (*accept) (socket_t *this);

    /**
     * @brief status_t 
     *
     * @param family        AF_INET, AF_INET6 and so on 
     * @param type          SOCK_STREAM, SOCK_DGRAM and so on
     * @param prototype     IPPROTO_UDP, IPPROTO_TCP and so on
     * @param ip            ip address of server
     * @param port          port of listening
     * @return
     *						- SUCCESS when packet successfully created
     *						- FAILED when unable to create
     */
    int (*connect) (socket_t *this, int family, int type, int prototype, char *ip, unsigned short port);

    /**
     * Receive a packet.
     *
     * Reads a packet from the socket and sets source/dest
     * appropriately.
     *
     * @param packet		pinter gets address from allocated packet_t
     * @return
     *						- SUCCESS when packet successfully received
     *						- FAILED when unable to receive
     */
    int (*receive)(socket_t *this, void *buf, int size, int timeout);

    /**
     * Send a packet.
     *
     * Sends a packet to the net using source and destination addresses of
     * the packet.
     *
     * @param packet		packet_t to send
     * @return
     *						- SUCCESS when packet successfully sent
     *						- FAILED when unable to send
     */
    int (*send)(socket_t *this, void *buf, int size);

    /**
     * Get the port this socket is listening on.
     *
     * @return				the port
     */
    unsigned short (*get_port)(socket_t *this);

    /**
     * Get the family this socket is listening on.
     *
     * @return				the family
     */
    unsigned short (*get_family)(socket_t *this);

    /**
     * Get the family this socket is listening on.
     *
     * @return				the family
     */
    struct sockaddr *(*get_sockaddr)(socket_t *this);

    /**
     * Get the family this socket is listening on.
     *
     * @return				the family
     */
    struct sockaddr *(*get_cli_sockaddr)(socket_t *this);

    /**
     * Get the client ip address this socket is listening on.
     *
     * @return				the client ip address
     */
    char *(*get_ip)(socket_t *this);

    /**
     * Get the client ip address this socket is listening on.
     *
     * @return				the client ip address
     */
    char *(*get_cli_ip)(socket_t *this);

    /**
     * Get the client port this socket is listening on.
     *
     * @return				the client port
     */
    unsigned short (*get_cli_port)(socket_t *this);

    /**
     * Get the socket type this socket is listening on.
     *
     * @return				the type
     */
    int (*get_type)(socket_t *this);

    /**
     * Get the socket instance this socket is listening on.
     *
     * @return				the socket instance
     */
    int (*get_sockfd)(socket_t *this);

    /**
     * Get the socket status this socket is listening on.
     *
     * @return				the status
     */
    int (*get_state)(socket_t *this);

    /**
     * Print the socket status this socket is listening on.
     *
     * @return				Print the status
     */
    void (*print_state)(socket_t *this);

    /**
     * Destroy a socket implementation.
     */
    void (*destroy)(socket_t *this);
};

/**
 * Create a socket_default_socket instance.
 */
socket_t *create_socket();

/**
 * Helper function to (un-)register socket interfaces from plugin features.
 *
 * This function is a plugin_feature_callback_t and can be used with the
 * PLUGIN_CALLBACK macro to register an socket interface constructor.
 *
 * @param plugin		plugin registering the socket interface
 * @param feature		associated plugin feature
 * @param reg			TRUE to register, FALSE to unregister
 * @param data			data passed to callback, a socket_constructor_t
 */
//bool socket_register(plugin_t *plugin, plugin_feature_t *feature,
//					 bool reg, void *data);

#endif /* __SOCKET_H__ */
