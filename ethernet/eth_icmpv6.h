#ifndef _ETH_ICMPV6_H
#define _ETH_ICMPV6_H

#include <eth_ipv6.h>

typedef struct __attribute__((__packed__))
{
  unsigned char  type;
  unsigned char  code;
  unsigned short checksum;
} ETH_ICMPV6_Header;

typedef struct __attribute__((__packed__))
{
  ETH_Header eth_hdr;
  ETH_IPV6_Header ipv6_hdr;
  ETH_ICMPV6_Header icmpv6_hdr;
} ETH_ICMPV6_FullHeader;

#define ETH_ICMPV6_HEADER_LENGTH 4

#define ETH_ICMPV6_TYPE_ECHO_REQUEST 128
#define ETH_ICMPV6_TYPE_ECHO_REPLY   129
#define ETH_ICMPV6_TYPE_RS           133 // Router Solicitation
#define ETH_ICMPV6_TYPE_RA           134 // Router Advertisement
#define ETH_ICMPV6_TYPE_NS           135 // Neighbor Solicitation
#define ETH_ICMPV6_TYPE_NA           136 // Neighbor Advertisement

void ETH_ICMPV6_Handler(const ETH_ICMPV6_FullHeader *icmp_hdr, unsigned int length);
void ETH_ICMPV6_MulticastHandler(const ETH_ICMPV6_FullHeader *icmp_hdr, unsigned int length);
unsigned short ETH_ICMPV6_ComputeChecksum(const ETH_ICMPV6_FullHeader *header);

#endif
