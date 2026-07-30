#ifndef _ETH_IPV6_H
#define _ETH_IPV6_H

#include <eth.h>

typedef struct __attribute__((__packed__))
{
  unsigned int version: 4;
  unsigned int trafficClass: 8;
  unsigned int flowLabel: 20;
  unsigned int payloadLength: 16;
  unsigned int nextHeader: 8;
  unsigned int hopLimit: 8;
  ETH_IPV6_Address sourceIP;
  ETH_IPV6_Address destIP;
} ETH_IPV6_Header;

typedef struct
{
  ETH_IPV6_Address sourceIP;
  ETH_IPV6_Address destIP;
} ETH_IPV6_Addresses;

#define ETH_IPV6_NEXT_HEADER_ICMPV6 58
#define ETH_IPV6_NEXT_HEADER_TCP    6
#define ETH_IPV6_NEXT_HEADER_UDP    17
#define ETH_IPV6_HEADER_LENGTH      40

void ETH_IPV6_Handler(const ETH_Header *eth_hdr, unsigned int length);
void ETH_IPV6_MulticastHandler(const ETH_Header *eth_hdr, unsigned int length);

#endif
