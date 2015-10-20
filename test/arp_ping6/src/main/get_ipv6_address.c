#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <ifaddrs.h>
#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), unsigned char, unsigned short
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_ICMPV6, INET6_ADDRSTRLEN
#include <netinet/ip.h>       // IP_MAXPACKET (which is 65535)
#include <netinet/ip6.h>      // struct ip6_hdr
#include <netinet/icmp6.h>    // struct icmp6_hdr and ICMP6_ECHO_REQUEST
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <sys/time.h>         // gettimeofday()

#include <errno.h>            // errno, perror()

#define ETH_HDRLEN  14  // Ethernet header length
#define IP6_HDRLEN  40  // IPv6 header length
#define ICMP_HDRLEN 8   // ICMP header length for echo request, excludes data

int get_ipv6_address(char *ipv6, const char *ifname)
{
    struct ifaddrs *ifaddr, *ifa;
    int    family, s;

    if ( ipv6 == NULL ) {
        return -1;
    }

    if ( getifaddrs(&ifaddr) == -1 ) {
        return -1;
    }

    for ( ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next ) {
        if ( ifa->ifa_addr == NULL ) {
            continue;
        }

        family = ifa->ifa_addr->sa_family;
        if ( family != AF_INET6 || strcasecmp(ifa->ifa_name, ifname) ) {
            continue;
        }

        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), ipv6,
                NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if ( s != 0 ) {
            return -1;
        }

        break;
    }

    freeifaddrs(ifaddr);
    return 0;
}

unsigned short checksum (unsigned short *addr, int len)
{
    int count = len;
    register uint32_t sum = 0;
    unsigned short answer = 0;

    // Sum up 2-byte values until none or only one byte left.
    while (count > 1) {
        sum += *(addr++);
        count -= 2;
    }

    // Add left-over byte, if any.
    if (count > 0) {
        sum += *(unsigned char *) addr;
    }

    // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    // Checksum is one's compliment of sum.
    answer = ~sum;

    return (answer);
}

unsigned short icmp6_checksum (struct ip6_hdr iphdr, struct icmp6_hdr icmp6hdr)
{
    char buf[IP_MAXPACKET];
    char *ptr;
    int chksumlen = 0;

    ptr = &buf[0];  // ptr points to beginning of buffer buf

    // Copy source IP address into buf (128 bits)
    memcpy (ptr, &iphdr.ip6_src.s6_addr, sizeof (iphdr.ip6_src.s6_addr));
    ptr       += sizeof (iphdr.ip6_src);
    chksumlen += sizeof (iphdr.ip6_src);

    // Copy destination IP address into buf (128 bits)
    memcpy (ptr, &iphdr.ip6_dst.s6_addr, sizeof (iphdr.ip6_dst.s6_addr));
    ptr       += sizeof (iphdr.ip6_dst.s6_addr);
    chksumlen += sizeof (iphdr.ip6_dst.s6_addr);

    // Copy Upper Layer Packet length into buf (32 bits).
    *ptr = 0; ptr++;
    *ptr = 0; ptr++;
    *ptr = (ICMP_HDRLEN) / 256;
    ptr++;
    *ptr = (ICMP_HDRLEN) % 256;
    ptr++;
    chksumlen += 4;

    // Copy zero field to buf (24 bits)
    *ptr = 0; ptr++;
    *ptr = 0; ptr++;
    *ptr = 0; ptr++;
    chksumlen += 3;

    // Copy next header field to buf (8 bits)
    memcpy (ptr, &iphdr.ip6_nxt, sizeof (iphdr.ip6_nxt));
    ptr       += sizeof (iphdr.ip6_nxt);
    chksumlen += sizeof (iphdr.ip6_nxt);

    // Copy ICMPv6 type to buf (8 bits)
    memcpy (ptr, &icmp6hdr.icmp6_type, sizeof (icmp6hdr.icmp6_type));
    ptr       += sizeof (icmp6hdr.icmp6_type);
    chksumlen += sizeof (icmp6hdr.icmp6_type);

    // Copy ICMPv6 code to buf (8 bits)
    memcpy (ptr, &icmp6hdr.icmp6_code, sizeof (icmp6hdr.icmp6_code));
    ptr       += sizeof (icmp6hdr.icmp6_code);
    chksumlen += sizeof (icmp6hdr.icmp6_code);

    // Copy ICMPv6 ID to buf (16 bits)
    memcpy (ptr, &icmp6hdr.icmp6_id, sizeof (icmp6hdr.icmp6_id));
    ptr       += sizeof (icmp6hdr.icmp6_id);
    chksumlen += sizeof (icmp6hdr.icmp6_id);

    // Copy ICMPv6 sequence number to buff (16 bits)
    memcpy (ptr, &icmp6hdr.icmp6_seq, sizeof (icmp6hdr.icmp6_seq));
    ptr       += sizeof (icmp6hdr.icmp6_seq);
    chksumlen += sizeof (icmp6hdr.icmp6_seq);

    // Copy ICMPv6 checksum to buf (16 bits)
    *ptr = 0; ptr++;
    *ptr = 0; ptr++;
    chksumlen += 2;

    return checksum ((unsigned short *) buf, chksumlen);
}

int arp_ping6(const char *target, int tries, const struct timeval *timeout, const char *ifname)
{
    int  status, frame_length;
    int  send_fd, recv_fd;
    int  done = 0;
    char src_ip[INET6_ADDRSTRLEN];
    char dst_ip[INET6_ADDRSTRLEN];
    char recv_ip[INET6_ADDRSTRLEN];
    void *tmp;
    unsigned char       src_mac[6], dst_mac[6];
    unsigned char       send_ether_frame[IP_MAXPACKET], recv_ether_frame[IP_MAXPACKET];
    struct ip6_hdr      send_iphdr,   *recv_iphdr;
    struct icmp6_hdr    send_icmphdr, *recv_icmphdr;
    struct addrinfo     hints, *res;
    struct sockaddr_in6 *ipv6;
    struct sockaddr_ll  device;
    struct ifreq        ifr;
    struct sockaddr     from;
    socklen_t           fromlen;
    fd_set              fdset;
    struct timeval      wait, start, end;

    // Submit request for a socket descriptor to look up ifname.
    if ((send_fd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
        return (EXIT_FAILURE);
    }

    // Use ioctl() to look up ifname name and get its MAC address.
    memset (&ifr, 0, sizeof (ifr));
    memcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    if (ioctl (send_fd, SIOCGIFHWADDR, &ifr) < 0) {
        return (EXIT_FAILURE);
    }

    // Copy source MAC address.
    memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6);

    // Find ifname index from interface name and store index in
    memset (&device, 0, sizeof (device));
    if ((device.sll_ifindex = if_nametoindex (ifname)) == 0) {
        return (EXIT_FAILURE);
    }

    // Set destination MAC address: you need to fill these out
    memset(dst_mac, 1, 6);

    // Source IPv6 address: you need to fill this out
    get_ipv6_address(src_ip, ifname);

    // Fill out hints for getaddrinfo().
    memset (&hints, 0, sizeof (hints));
    hints.ai_family   = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = hints.ai_flags | AI_CANONNAME;

    // Resolve target using getaddrinfo().
    if ((status = getaddrinfo (target, NULL, &hints, &res)) != 0) {
        return (EXIT_FAILURE);
    }
    ipv6 = (struct sockaddr_in6 *) res->ai_addr;
    tmp  = &(ipv6->sin6_addr);
    if (inet_ntop (AF_INET6, tmp, dst_ip, INET6_ADDRSTRLEN) == NULL) {
        return (EXIT_FAILURE);
    }
    freeaddrinfo (res);

    // Fill out sockaddr_ll.
    device.sll_family = AF_PACKET;
    device.sll_halen  = 6;
    memcpy (device.sll_addr, src_mac, 6);

    // IPv6 header
    send_iphdr.ip6_flow = htonl ((6 << 28));
    send_iphdr.ip6_plen = htons (ICMP_HDRLEN);
    send_iphdr.ip6_nxt  = IPPROTO_ICMPV6;
    send_iphdr.ip6_hops = 255;

    // Source IPv6 address (128 bits)
    if ((status = inet_pton (AF_INET6, src_ip, &(send_iphdr.ip6_src))) != 1) {
        return (EXIT_FAILURE);
    }

    // Destination IPv6 address (128 bits)
    if ((status = inet_pton (AF_INET6, dst_ip, &(send_iphdr.ip6_dst))) != 1) {
        return (EXIT_FAILURE);
    }

    // ICMP header
    send_icmphdr.icmp6_type  = ICMP6_ECHO_REQUEST;
    send_icmphdr.icmp6_code  = 0;
    send_icmphdr.icmp6_id    = htons (1000);
    send_icmphdr.icmp6_seq   = htons (0);
    send_icmphdr.icmp6_cksum = icmp6_checksum (send_iphdr, send_icmphdr);

    // Fill out ethernet frame header.
    frame_length = 6 + 6 + 2 + IP6_HDRLEN + ICMP_HDRLEN;
    memcpy (send_ether_frame,     dst_mac, 6);
    memcpy (send_ether_frame + 6, src_mac, 6);
    send_ether_frame[12] = ETH_P_IPV6 / 256;
    send_ether_frame[13] = ETH_P_IPV6 % 256;

    // Next is ethernet frame data (IPv6 header + ICMP header + ICMP data).
    memcpy (send_ether_frame + ETH_HDRLEN, &send_iphdr, IP6_HDRLEN);
    memcpy (send_ether_frame + ETH_HDRLEN + IP6_HDRLEN, &send_icmphdr, ICMP_HDRLEN);

    // Submit request for a raw socket descriptor to receive packets.
    if ((recv_fd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
        return (EXIT_FAILURE);
    }

    // Cast recv_iphdr and recv_icmphdr as pointer to ethernet frame.
    recv_iphdr   = (struct ip6_hdr *) (recv_ether_frame + ETH_HDRLEN);
    recv_icmphdr = (struct icmp6_hdr *) (recv_ether_frame + ETH_HDRLEN + IP6_HDRLEN);

    while (tries-- > 0) {
        // SEND
        if (sendto (send_fd, send_ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device)) <= 0) {
            return (EXIT_FAILURE);
        }

        // Start timer
        gettimeofday(&start, NULL);

        // RECEIVE LOOP
        memcpy(&wait, timeout, sizeof(wait));
        while (!done) {
            memset (recv_ether_frame, 0, IP_MAXPACKET);
            memset (&from, 0, sizeof (from));
            fromlen = sizeof (from);

            FD_ZERO(&fdset);
            FD_SET(recv_fd, &fdset);
            status = select(recv_fd + 1, &fdset, NULL, NULL, &wait);
            switch (status) {
                case 0:
                    break;
                case -1:
                    return (EXIT_FAILURE);
                default:
                    if (FD_ISSET(recv_fd, &fdset)) {
                        if (recvfrom (recv_fd, recv_ether_frame, IP_MAXPACKET, 0, (struct sockaddr *) &from, &fromlen) < 0) {
                            continue;
                        } 

                        // Check for an IP ethernet frame, carrying ICMP echo reply. If not, ignore and keep listening.
                        if ((((recv_ether_frame[12] << 8) + recv_ether_frame[13]) == ETH_P_IPV6) &&
                                (recv_iphdr->ip6_nxt == IPPROTO_ICMPV6) && (recv_icmphdr->icmp6_type == ICMP6_ECHO_REPLY) && (recv_icmphdr->icmp6_code == 0)) {

                            // Extract source IP address from received ethernet frame.
                            if (inet_ntop (AF_INET6, &(recv_iphdr->ip6_src), recv_ip, INET6_ADDRSTRLEN) == NULL) {
                                return (EXIT_FAILURE);
                            }

                            if (!strcasecmp(recv_ip, target)) {
                                printf ("%s (received)\n", recv_ip);
                                done = 1;
                                break;  // Break out of Receive loop.
                            }

                        }  // End if IP ethernet frame carrying ICMP_ECHOREPLY
                    }            
            }

            // calculate how long it took
            gettimeofday(&end, NULL);
            if ((int)((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) > (int)(timeout->tv_sec * 1000000 + timeout->tv_usec)) {
                break;
            }
        } // End of Receive loop.

        // an echo reply was received; break out of send loop.
        if (done == 1) {
            break;  // Break out of Send loop.
        }

    }  // End of Send loop.

    // Close socket.
    close (send_fd);
    close (recv_fd);

    return (EXIT_SUCCESS);
}

int main (int argc, char **argv)
{
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    arp_ping6(argv[1], 3, &timeout, "eth0");
    return 0;
}
