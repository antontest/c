#ifndef __SOCKET_BASE__
#define __SOCKET_BASE__
#include <sys/socket.h>

typedef struct socket_base_t socket_base_t;
struct socket_base_t {
    /**
     * @brief  Creates an endpoint for communication and returns a descriptor.
     *
     * @param family  The  domain argument specifies a communication domain;
     *                Name                Purpose                          Man page
     *                AF_UNIX, AF_LOCAL   Local communication              unix(7)
     *                AF_INET             IPv4 Internet protocols          ip(7)
     *                AF_INET6            IPv6 Internet protocols          ipv6(7)
     *                AF_IPX              IPX - Novell protocols           
     *                AF_NETLINK          Kernel user interface device     netlink(7)
     *                AF_X25              ITU-T X.25 / ISO-8208 protocol   x25(7)
     *                AF_AX25             Amateur radio AX.25 protocol 
     *                AF_ATMPVC           Access to raw ATM PVCs
     *                AF_APPLETALK        Appletalk                        ddp(7)
     *                AF_PACKET           Low level packet interface       packet(7)
     *
     * @param type     Specifies the communication semantics.  Currently defined types are:
     *                 SOCK_STREAM     Provides sequenced, reliable, two-way, connection-based byte  streams.   An  out-of-band  data
                       transmission mechanism may be supported.
     *                 SOCK_DGRAM      Supports datagrams (connectionless, unreliable messages of a fixed maximum length).
     *                 SOCK_SEQPACKET  Provides  a sequenced, reliable, two-way connection-based data transmission path for datagrams
                       of fixed maximum length; a consumer is required to read an entire packet with each input  sys‐
                       tem call.
     *                 SOCK_RAW        Provides raw network protocol access.
     *                 SOCK_RDM        Provides a reliable datagram layer that does not guarantee ordering.
                       SOCK_PACKET     Obsolete and should not be used in new programs; see packet(7).

     * @param protocol The  protocol  specifies  a  particular  protocol to be used with the socket.
     * Normally only a single protocol exists to support a particular socket type within a given protocol family, in which case protocol can be spec‐
       ified  as  0.  However, it is possible that many protocols may exist, in which case a particular protocol must
       be specified in this manner.  The protocol number to use is specific to the “communication  domain”  in  which
       communication  is  to take place; see protocols(5).  See getprotoent(3) on how to map protocol name strings to
       protocol numbers.

     * @return On success, a file descriptor for the new socket is returned. On error, -1 is returned, and errno is set appropriately.
     */
    int (*socket) (socket_base_t *this, int family, int type, int protocol);

    /**
     * @brief  Initial socket address information
     */
    struct sockaddr *(*init_addr) (socket_base_t *this, int family, char *ip, unsigned int port);

    /**
     * @brief Assigns the address specified to by addr to the socket referred to by the file descriptor sockfd.
     *
     * @param ip    IP address binding
     * @param port  Port binding
     * @return      On success, zero is returned.  On error, -1 is returned, and  errno is set appropriately.
     */
    int (*bind) (socket_base_t *this, char *ip, unsigned int port);

    /**
     * @brief marks the socket referred to by sockfd as a passive socket, that is, as a socket that will be used to
     *        accept incoming connection requests using accept(2).
     *
     * @param backlog  The backlog argument defines the maximum length to which
     *                 the queue of pending connections for sockfd may grow.
     *                 If  a connection request arrives when the queue is full, the
     *                 client may receive an error with an indication of
     *                 ECONNREFUSED or, if the underlying protocol supports
     *                 retransmission, the request may  be  ignored  so  that  a
     *                 later reattempt at connection succeeds.
     *
     * @return         On success, zero is returned.  On error, -1 is returned,
     *                 and errno is set appropriately.
     */
    int (*listen) (socket_base_t *this, int backlog);

    /**
     * @brief It extracts the first connection request on the queue of pending
     *        connections for the  listening  socket,
     *
     * @param addr  The  argument  addr is a pointer to a sockaddr structure.
     *              This structure is filled in with the address of the
     *              peer socket, as known to the communications layer.  The exact
     *              format of the address returned  addr  is  deter‐
     *              mined  by  the  socket's  address  family (see socket(2)
     *              and the respective protocol man pages).  When addr is
     *              NULL, nothing is filled in;
     *
     * @return      On success, these system calls return a non-negative integer
     *              that is a descriptor for the accepted socket.  On
     *              error, -1 is returned, and errno is set appropriately.
     */
    int (*accept) (socket_base_t *this, struct sockaddr *addr);

    /**
     * @brief Connects the socket referred to by the file descriptor sockfd to
     *        the address specified by ip and port.
     *
     * @param ip    Server IP address
     * @param port  Server port listening on
     * 
     * @return      If the connection or binding succeeds, zero is returned.  On
     *              error, -1 is returned, and errno is set appropriately.
     */
    int (*connect) (socket_base_t *this, char *ip, unsigned int port);

    /**
     * @brief  The  send()  call  may be used only when the socket is in a
     *         connected state (so that the intended recipient is
     *         known).  The only difference between send() and write(2) is the
     *         presence of flags.  With zero flags  argument,
     *         send() is equivalent to write(2). Also, the following call
     *              send(sockfd, buf, len, flags);
     *         is equivalent to
     *              sendto(sockfd, buf, len, flags, NULL, 0);
     *
     * @param buf   If the message is too long to pass atomically through the
     *              underlying protocol, the error EMSGSIZE is returned,
     *              and the message is not transmitted.
     * @param size  size of sending messages
     * @param flags The flags argument is the bitwise OR of zero or more of the following flags.

     *  MSG_CONFIRM (Since Linux 2.3.15)
              Tell the link layer that forward progress happened: you got a successful reply from the other side.  If
              the link layer doesn't get this it will regularly reprobe the neighbor (e.g., via a unicast ARP).  Only
              valid  on SOCK_DGRAM and SOCK_RAW sockets and currently only implemented for IPv4 and IPv6.  See arp(7)
              for details.

     *  MSG_DONTROUTE
              Don't use a gateway to send out the packet, only send to hosts on directly connected networks.  This is
              usually  used  only by diagnostic or routing programs.  This is only defined for protocol families that
              route; packet sockets don't.

     *  MSG_DONTWAIT (since Linux 2.2)
              Enables non-blocking operation; if the operation would block, EAGAIN or EWOULDBLOCK is  returned  (this
              can also be enabled using the O_NONBLOCK flag with the F_SETFL fcntl(2)).

     *  MSG_EOR (since Linux 2.2)
              Terminates a record (when this notion is supported, as for sockets of type SOCK_SEQPACKET).

     *  MSG_MORE (Since Linux 2.4.4)
              The  caller has more data to send.  This flag is used with TCP sockets to obtain the same effect as the
              TCP_CORK socket option (see tcp(7)), with the difference that this flag can be set on a per-call basis.

              Since Linux 2.6, this flag is also supported for UDP sockets, and informs the kernel to package all  of
              the  data sent in calls with this flag set into a single datagram which is only transmitted when a call
              is performed that does not specify this flag.  (See  also  the  UDP_CORK  socket  option  described  in
              udp(7).)

     *  MSG_NOSIGNAL (since Linux 2.2)
              Requests not to send SIGPIPE on errors on stream oriented sockets when the other end breaks the connec‐
              tion.  The EPIPE error is still returned.

     *  MSG_OOB
              Sends out-of-band data on sockets that support this notion (e.g., of type SOCK_STREAM); the  underlying
              protocol must also support out-of-band data.
     *
     * @return  On  success,  these  calls  return  the number of characters sent.  On error, -1 is returned, and errno is set
                appropriately.    
     */
    int (*send) (socket_base_t *this, void *buf, int size, int flags);
    int (*sendto) (socket_base_t *this, void *buf, int size, int flags, struct sockaddr *dest_addr, int addrlen);

    /**
     * @brief Receive messages from a socket, and may be used to receive data
       	      on a socket whether or not it is connection-oriented.
   


     * @param buf    If no messages are available at the socket, the receive calls wait for a message to arrive, unless the  socket
                     is  non-blocking (see fcntl(2)), in which case the value -1 is returned and the external variable errno is set
                     to EAGAIN or EWOULDBLOCK.  The receive calls normally return any data available, up to the  requested  amount,
                     rather than waiting for receipt of the full amount requested.
     * @param size   size of recving buffer
     * @param flags  The flags argument to a recv() call is formed by OR'ing one or more of the following values:

     * MSG_CMSG_CLOEXEC (recvmsg() only; since Linux 2.6.23)
              Set the close-on-exec flag for the file descriptor received via a Unix domain file descriptor using the
              SCM_RIGHTS operation (described in unix(7)).  This flag is useful for the same reasons as the O_CLOEXEC
              flag of open(2).

     * MSG_DONTWAIT (since Linux 2.2)
              Enables  non-blocking  operation; if the operation would block, the call fails with the error EAGAIN or
              EWOULDBLOCK (this can also be enabled using the O_NONBLOCK flag with the F_SETFL fcntl(2)).

     * MSG_ERRQUEUE (since Linux 2.2)
              This flag specifies that queued errors should be received from the socket error queue.   The  error  is
              passed  in  an ancillary message with a type dependent on the protocol (for IPv4 IP_RECVERR).  The user
              should supply a buffer of sufficient size.  See cmsg(3) and ip(7) for more information.  The payload of
              the  original packet that caused the error is passed as normal data via msg_iovec.  The original desti‐
              nation address of the datagram that caused the error is supplied via msg_name.

              For local errors, no address is passed (this can be checked with the cmsg_len member of  the  cmsghdr).
              For error receives, the MSG_ERRQUEUE is set in the msghdr.  After an error has been passed, the pending
              socket error is regenerated based on the next queued error and will be passed on the next socket opera‐
              tion.

      * MSG_OOB
              This  flag  requests  receipt of out-of-band data that would not be received in the normal data stream.
              Some protocols place expedited data at the head of the normal data queue, and thus this flag cannot  be
              used with such protocols.

     * MSG_PEEK
              This  flag  causes the receive operation to return data from the beginning of the receive queue without
              removing that data from the queue.  Thus, a subsequent receive call will return the same data.

     * MSG_TRUNC (since Linux 2.2)
              For raw (AF_PACKET), Internet datagram (since Linux 2.4.27/2.6.8), and  netlink  (since  Linux  2.6.22)
              sockets: return the real length of the packet or datagram, even when it was longer than the passed buf‐
              fer.  Not implemented for Unix domain (unix(7)) sockets.

              For use with Internet stream sockets, see tcp(7).

     * MSG_WAITALL (since Linux 2.2)
              This flag requests that the operation block until the full request is satisfied.  However, the call may
              still return less data than requested if a signal is caught, an error or disconnect occurs, or the next
              data to be received is of a different type than that returned.

     * @return   These  calls return the number of bytes received, or -1 if an error occurred.  The return value will be 0 when
                 the peer has performed an orderly shutdown.
     */
    int (*recv) (socket_base_t *this, void *buf, int size, int flags);
    int (*recvfrom) (socket_base_t *this, void *buf, int size, int flags, struct sockaddr *src_addr, int *addrlen);

    /**
     * @brief Close a file descriptor 
              close() closes a file descriptor, so that it no longer refers to any file and may be reused.  Any record locks
              (see fcntl(2)) held on the file it was associated with, and owned by the process, are removed  (regardless  of
              the file descriptor that was used to obtain the lock).

              If  fd  is  the  last  file  descriptor  referring  to the underlying open file description (see open(2)), the
              resources associated with the open file description are freed; if the descriptor was the last reference  to  a
              file which has been removed using unlink(2) the file is deleted.
     *
     * @return returns zero on success.  On error, -1 is returned, and errno is set appropriately. 
     */
    int (*close) (socket_base_t *this, int fd);

    /**
     * @brief  Shut down part of a full-duplex connection
               The  shutdown() call causes all or part of a full-duplex connection on the socket associated with sockfd to be
               shut down.
     * @param how  How to shutdown connection
                   If how is SHUT_RD, further receptions will be disallowed.  
                   If how is  SHUT_WR,  further  transmissions will be disallowed.  
                   If how is SHUT_RDWR, further receptions and transmissions will be disallowed.

     * @return     On success, zero is returned.  On error, -1 is returned, and errno is set appropriately. 
     */
    int (*shutdown) (socket_base_t *this, int fd, int how);

    /**
     * @brief destroy instance and free memory 
     */
    void (*destroy) (socket_base_t *this);
    
    /**
     * @brief Get socket descriptor
     */
    int (*get_fd) (socket_base_t *this);
};

/**
 * @brief Create socket_base instance
 */
socket_base_t *create_socket_base();

#endif /* __SOCKET_BASE__ */
