#ifndef __SOCKET_ROUTER_H__
#define __SOCKET_ROUTER_H__
#include <stdio.h>
#include <string.h>  
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>   
#include <arpa/inet.h>    
#include <netdb.h>
#include <netinet/if_ether.h> 
#include <unistd.h> 
#include <net/if_arp.h>
#include <stdlib.h>
#include "socket_property.h"
#include "socket_header.h"

int route(char *dstip);

#endif
