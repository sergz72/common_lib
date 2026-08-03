#ifndef _ETH_UDP_H
#define _ETH_UDP_H

#include <eth_ipv6.h>
#include <queue.h>

typedef struct __attribute__((__packed__))
{
  unsigned short source_port;
  unsigned short dest_port;
  unsigned short length;
  unsigned short checksum;
} ETH_UDP_Header;

typedef struct __attribute__((__packed__))
{
  ETH_Header eth_hdr;
  ETH_IPV6_Header ipv6_hdr;
  ETH_UDP_Header udp_hdr;
} ETH_UDP_FullHeader;

#define ETH_UDP_HEADER_LENGTH 8

void ETH_UDP_Handler(const ETH_UDP_FullHeader *udp_hdr);
int ETH_UDP_Send(unsigned short sourcePort, const unsigned char *destIP, unsigned short destPort, const void *data,
                 unsigned int data_length, Queue *q);
unsigned short ETH_UDP_ComputeChecksum(const ETH_UDP_FullHeader *header);

#endif
