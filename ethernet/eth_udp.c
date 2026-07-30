#include <eth_udp.h>
#include <eth_ndp.h>
#include <memory.h>
#include <eth_queue.h>

void ETH_UDP_Handler(const ETH_UDP_FullHeader *udp_hdr)
{

}

int ETH_UDP_Send(unsigned short sourcePort, const ETH_IPV6_Address *destIP, unsigned short destPort, void *data, unsigned int data_length)
{
  unsigned int i;
  ETH_UDP_FullHeader *header = (ETH_UDP_FullHeader *)queue_peekTx(&eth_user_queue);
  const unsigned char *hwAddr = ETH_GetHWAddr(destIP);

  if (hwAddr == nullptr)
    return 1;

  for (i = 0; i < ETH_HWADDR_LEN; i++)
  {
    header->eth_hdr.src_addr[i] = eth_instance.mac_address[i];
    header->eth_hdr.dest_addr[i] = *hwAddr++;
  }
  header->eth_hdr.type = ETH_PROTOCOL_IPV6;
  header->ipv6_hdr.nextHeader = ETH_IPV6_NEXT_HEADER_UDP;
  header->ipv6_hdr.version = 6;
  header->ipv6_hdr.payloadLength = ETH_SwapShort(ETH_UDP_HEADER_LENGTH + data_length);
  header->ipv6_hdr.trafficClass = 0;
  header->ipv6_hdr.flowLabel = 0;
  header->ipv6_hdr.hopLimit = 255;
  memcpy(&header->ipv6_hdr.destIP, destIP, sizeof(ETH_IPV6_Address));
  memcpy(&header->ipv6_hdr.sourceIP, &eth_instance.ipv6_address, sizeof(ETH_IPV6_Address));
  header->udp_hdr.source_port = sourcePort;
  header->udp_hdr.dest_port = destPort;
  header->udp_hdr.length = ETH_SwapShort(ETH_UDP_HEADER_LENGTH + data_length);
  header->udp_hdr.checksum = 0; // disabled

  memcpy((unsigned char*)header + sizeof(ETH_UDP_FullHeader), data, data_length);

  queue_add(&eth_user_queue, data_length + sizeof(ETH_UDP_FullHeader));

  return 0;
}
