#ifndef __LOCAL_H__
#define __LOCAL_H__
#include <sys/types.h>
#include <sys/socket.h>

typedef struct local_socket_t local_socket_t;
struct local_socket_t {
    /**
     * @brief  Creates an endpoint for communication and returns a descriptor.
     *
     * @param type     Specifies the communication semantics.  Currently defined types are:
     *                 SOCK_STREAM     Provides sequenced, reliable, two-way, connection-based byte  streams.   An  out-of-band  data
                       transmission mechanism may be supported.
     *                 SOCK_DGRAM      Supports datagrams (connectionless, unreliable messages of a fixed maximum length).
     * @return On success, a file descriptor for the new socket is returned. On error, -1 is returned, and errno is set appropriately.
     */
    int (*socket) (local_socket_t *this, int type);

    /**
     * @brief  Initial socket address information
     */
    struct sockaddr *(*init_addr) (local_socket_t *this, const char *path);

    /**
     * @brief Assigns the address specified to by addr to the socket referred to by the file descriptor sockfd.
     * @return      On success, zero is returned.  On error, -1 is returned, and  errno is set appropriately.
     */
    int (*bind) (local_socket_t *this);

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
    int (*listen) (local_socket_t *this, int backlog);

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
    int (*accept) (local_socket_t *this, struct sockaddr *addr);

    /**
     * @brief Connects the socket referred to by the file descriptor sockfd to
     *        the address specified by ip and port.
     * @return      If the connection or binding succeeds, zero is returned.  On
     *              error, -1 is returned, and errno is set appropriately.
     */
    int (*connect) (local_socket_t *this);

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
     * @return  On  success,  these  calls  return  the number of characters sent.  On error, -1 is returned, and errno is set
                appropriately.    
     */
    int (*send) (local_socket_t *this, void *buf, int size, int flags);
    int (*sendto) (local_socket_t *this, void *buf, int size, int flags, struct sockaddr *addr);

    /**
     * @brief Receive messages from a socket, and may be used to receive data
       	      on a socket whether or not it is connection-oriented.
     * @param buf    If no messages are available at the socket, the receive calls wait for a message to arrive, unless the  socket
                     is  non-blocking (see fcntl(2)), in which case the value -1 is returned and the external variable errno is set
                     to EAGAIN or EWOULDBLOCK.  The receive calls normally return any data available, up to the  requested  amount,
                     rather than waiting for receipt of the full amount requested.
     * @param size   size of recving buffer
     * @param flags  The flags argument to a recv() call is formed by OR'ing one or more of the following values:
     * @return   These  calls return the number of bytes received, or -1 if an error occurred.  The return value will be 0 when
                 the peer has performed an orderly shutdown.
     */
    int (*recv) (local_socket_t *this, void *buf, int size, int flags);
    int (*recvfrom) (local_socket_t *this, void *buf, int size, int flags, struct sockaddr *addr);

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
    int (*close) (local_socket_t *this);


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
    int (*shutdown) (local_socket_t *this, int how);

    /**
     * @brief destroy instance and free memory 
     */
    void (*destroy) (local_socket_t *this);
    
    /**
     * @brief Get socket descriptor
     */
    int (*get_fd) (local_socket_t *this);
    int (*get_accepted_fd) (local_socket_t *this);
};

/**
 * @brief create local socket instance 
 */
local_socket_t *create_local_socket();

#endif /* __LOCAL_H__ */
