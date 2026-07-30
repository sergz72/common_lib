#include <eth.h>
#include <eth_ipv6.h>
#include <string.h>
#include <eth_queue.h>
#include <eth_ndp.h>
#include <stdio.h>
#include <trng.h>

static const unsigned char multicast_ipv6_mac_address[6] = { 0x33, 0x33, 0, 0, 0, 1 };

ETH_Instance eth_instance;

void ETH_Init(const unsigned char *mac, int printf_func ( const char * format, ... ), bool debug)
{
  memset(&eth_instance, 0, sizeof(ETH_Instance));
  eth_instance.debug = debug;
  eth_instance.printf_func = printf_func;
  memcpy(eth_instance.mac_address, mac, 6);
  trng_generate((unsigned int*)&eth_instance.ipv6_address, 4);
  ETH_QueueInit();
  ETH_InitNdpTable();
}

void ethernet_packet_received(const void *buffer, const unsigned int length)
{
  const ETH_Header *eth_hdr = buffer;

  if (eth_instance.debug)
  {
    const unsigned char *p = buffer;
    for (int i = 0; i < length; i++)
      eth_instance.printf_func("%02X ", *p++);
    eth_instance.printf_func("\n");
  }
  switch (eth_hdr->type)
  {
    case ETH_PROTOCOL_IPV6:
      if (!memcmp(eth_hdr->dest_addr, multicast_ipv6_mac_address, 6))
        ETH_IPV6_MulticastHandler(eth_hdr, length);
      else if (!memcmp(eth_hdr->dest_addr, eth_instance.mac_address, 6))
        ETH_IPV6_Handler(eth_hdr, length);
      break;
    default:
      break;
  }
}

void ETH_Set_Prefix(const unsigned char *prefix, unsigned char prefix_length, const unsigned char *router_mac)
{
  prefix_length /= 8;
  char *current_ip = (char*)&eth_instance.ipv6_address;
  if (!memcmp(current_ip, prefix, prefix_length))
    return;
  memcpy(&eth_instance.router_mac_address, router_mac, 6);
  memcpy(current_ip, prefix, prefix_length);
  print_ipv6_raw("My IP", current_ip, printf);
}

void print_ipv6_raw(const char *title, const unsigned char *ip)
{
 eth_instance.printf_func("%s: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
         title,
         ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7],
         ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
}
