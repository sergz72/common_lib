#include <eth.h>
#include <eth_ipv6.h>
#include <eth_udp.h>
#include <eth_icmpv6.h>
#include <eth_ndp.h>
#include <string.h>

void ETH_IPV6_MulticastHandler(const ETH_Header *eth_hdr, unsigned int length)
{
  const ETH_IPV6_Header *ipv6_header = (const ETH_IPV6_Header *)((const unsigned char*)eth_hdr + ETH_HEADER_LENGTH);

  ETH_UpdateNdpTable(ipv6_header->sourceIP, eth_hdr->src_addr);

  switch (ipv6_header->nextHeader)
  {
    case ETH_IPV6_NEXT_HEADER_ICMPV6:
      ETH_ICMPV6_MulticastHandler((const ETH_ICMPV6_FullHeader *)eth_hdr, length);
      break;
    default:
      break;
  }
}

void ETH_IPV6_Handler(const ETH_Header *eth_hdr, unsigned int length)
{
  const ETH_IPV6_Header *ipv6_header = (const ETH_IPV6_Header *)((const unsigned char*)eth_hdr + ETH_HEADER_LENGTH);

  ETH_UpdateNdpTable(ipv6_header->sourceIP, eth_hdr->src_addr);

  if (!memcmp(ipv6_header->destIP, eth_instance.ipv6_address, 16))
  {
    switch (ipv6_header->nextHeader)
    {
      case ETH_IPV6_NEXT_HEADER_UDP:
        ETH_UDP_Handler((const ETH_UDP_FullHeader *)eth_hdr);
        break;
      case ETH_IPV6_NEXT_HEADER_ICMPV6:
        ETH_ICMPV6_Handler((const ETH_ICMPV6_FullHeader *)eth_hdr, length);
        break;
      default:
        break;
    }
  }
}
