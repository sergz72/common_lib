#include <eth_udp.h>
#include <eth_ndp.h>
#include <eth_ntp.h>
#include <memory.h>
#include <eth_queue.h>
//#include "board.h"

static void send_echo(const ETH_UDP_FullHeader *udp_hdr)
{
  unsigned int i;
  ETH_UDP_FullHeader *header = (ETH_UDP_FullHeader *)queue_peekTx(&eth_irq_queue);

  memcpy(header->eth_hdr.src_addr, udp_hdr->eth_hdr.dest_addr, 16);
  memcpy(header->eth_hdr.dest_addr, udp_hdr->eth_hdr.src_addr, 16);
  header->eth_hdr.type = ETH_PROTOCOL_IPV6;
  header->ipv6_hdr.nextHeader = ETH_IPV6_NEXT_HEADER_UDP;
  header->ipv6_hdr.version_trafficClass_flowLabel_high = udp_hdr->ipv6_hdr.version_trafficClass_flowLabel_high;
  header->ipv6_hdr.payloadLength = udp_hdr->ipv6_hdr.payloadLength;
  header->ipv6_hdr.flowLabel_low = udp_hdr->ipv6_hdr.flowLabel_low;
  header->ipv6_hdr.hopLimit = 255;
  memcpy(header->ipv6_hdr.destIP, udp_hdr->ipv6_hdr.sourceIP, 16);
  memcpy(header->ipv6_hdr.sourceIP, udp_hdr->ipv6_hdr.destIP, 16);
  header->udp_hdr.source_port = udp_hdr->udp_hdr.dest_port;
  header->udp_hdr.dest_port = udp_hdr->udp_hdr.source_port;
  header->udp_hdr.length = udp_hdr->udp_hdr.length;
  header->udp_hdr.checksum = udp_hdr->udp_hdr.checksum;
  unsigned int data_length = ETH_SwapShort(udp_hdr->ipv6_hdr.payloadLength) - ETH_UDP_HEADER_LENGTH;
  memcpy((unsigned char*)header + sizeof(ETH_UDP_FullHeader), (unsigned char*)udp_hdr + sizeof(ETH_UDP_FullHeader), data_length);

  //header->udp_hdr.checksum =
  //  ETH_UDP_ComputeChecksum(header);

  queue_add(&eth_irq_queue, data_length + sizeof(ETH_UDP_FullHeader));
}

void ETH_UDP_Handler(const ETH_UDP_FullHeader *udp_hdr)
{
  //LED_RED_ON;
  if (udp_hdr->udp_hdr.source_port == NTP_PORT_REVERSED)
    ETH_NTP_Process_Timestamp_Reply((const unsigned char*)udp_hdr + sizeof(ETH_UDP_FullHeader));
  else if (udp_hdr->udp_hdr.dest_port == PORT_ECHO_REVERSED)
    send_echo(udp_hdr);
}

int ETH_UDP_Send(unsigned short sourcePort, const unsigned char *destIP, unsigned short destPort, const void *data,
                 unsigned int data_length, Queue *q)
{
  const unsigned char *myip = ETH_GetMyIP(destIP);
  unsigned int i;
  ETH_UDP_FullHeader *header = (ETH_UDP_FullHeader *)queue_peekTx(q);
  const unsigned char *hwAddr = ETH_GetHWAddr(destIP);

  if (hwAddr == nullptr)
    return 1;

  for (i = 0; i < ETH_HWADDR_LEN; i++)
  {
    header->eth_hdr.src_addr[i] = eth_instance.mac_address[i];
    header->eth_hdr.dest_addr[i] = *hwAddr++;
  }
  const unsigned short l = ETH_SwapShort(ETH_UDP_HEADER_LENGTH + data_length);
  header->eth_hdr.type = ETH_PROTOCOL_IPV6;
  header->ipv6_hdr.nextHeader = ETH_IPV6_NEXT_HEADER_UDP;
  header->ipv6_hdr.version_trafficClass_flowLabel_high = 0x60;
  header->ipv6_hdr.payloadLength = l;
  header->ipv6_hdr.flowLabel_low = 0;
  header->ipv6_hdr.hopLimit = 255;
  memcpy(header->ipv6_hdr.destIP, destIP, 16);
  memcpy(header->ipv6_hdr.sourceIP, myip, 16);
  header->udp_hdr.source_port = ETH_SwapShort(sourcePort);
  header->udp_hdr.dest_port = ETH_SwapShort(destPort);
  header->udp_hdr.length = l;
  header->udp_hdr.checksum = 0;
  memcpy((unsigned char*)header + sizeof(ETH_UDP_FullHeader), data, data_length);

  header->udp_hdr.checksum =
    ETH_UDP_ComputeChecksum(header);

  queue_add(q, data_length + sizeof(ETH_UDP_FullHeader));

  return 0;
}

unsigned short ETH_UDP_ComputeChecksum(const ETH_UDP_FullHeader *header)
{
  unsigned int sum = calculate_sum((unsigned short*)&header->ipv6_hdr.sourceIP, 32);
  sum += ETH_IPV6_NEXT_HEADER_UDP;
  unsigned int l = ETH_SwapShort(header->ipv6_hdr.payloadLength);
  sum += l;
  sum += calculate_sum((unsigned short*)&header->udp_hdr, l);

  // Fold 32-bit sum to 16 bits
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  unsigned short checksum = (unsigned short)~sum;

  // IPv6 UDP Rule: If checksum is 0, return 0xFFFF
  if (checksum == 0) {
    return 0xFFFF;
  }

  return ETH_SwapShort(checksum);
}
