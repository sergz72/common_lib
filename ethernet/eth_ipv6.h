#ifndef _ETH_IPV6_H
#define _ETH_IPV6_H

#include <eth.h>

typedef struct __attribute__((__packed__))
{
  unsigned short version_trafficClass_flowLabel_high;
  unsigned short flowLabel_low;
  unsigned short payloadLength;
  unsigned char nextHeader;
  unsigned char hopLimit;
  unsigned char sourceIP[16];
  unsigned char destIP[16];
} ETH_IPV6_Header;

#define ETH_IPV6_NEXT_HEADER_ICMPV6 58
#define ETH_IPV6_NEXT_HEADER_TCP    6
#define ETH_IPV6_NEXT_HEADER_UDP    17
#define ETH_IPV6_HEADER_LENGTH      40

void ETH_IPV6_Handler(const ETH_Header *eth_hdr, unsigned int length);
void ETH_IPV6_MulticastHandler(const ETH_Header *eth_hdr, unsigned int length);

#endif
