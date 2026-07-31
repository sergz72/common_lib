#include <eth.h>
#include <eth_ipv6.h>
#include <string.h>
#include <eth_queue.h>
#include <eth_ndp.h>
#include <trng.h>
#include <eth_ntp.h>
#include <stdlib.h>

static const unsigned char multicast_ipv6_mac_address[6] = { 0x33, 0x33, 0, 0, 0, 1 };
const unsigned char zero_ipv6_address[16] = {0};

ETH_Instance eth_instance;

void ETH_Init(const unsigned char *mac, const unsigned char *ntp_server_address, int printf_func ( const char * format, ... ), bool debug)
{
  memset(&eth_instance, 0, sizeof(ETH_Instance));
  if (ntp_server_address)
    memcpy(&eth_instance.ntp_server_address, ntp_server_address, 16);
  eth_instance.debug = debug;
  eth_instance.printf_func = printf_func;
  memcpy(eth_instance.mac_address, mac, 6);
  trng_generate((unsigned int*)&eth_instance.ipv6_address, 4);
  ETH_QueueInit();
  ETH_InitNdpTable();
  ETH_NTP_Init();
}

int ETH_Parse_IPV6(const unsigned char *address, unsigned char *result)
{
  unsigned short ip_address[8] = {0};
  int i;
  for (i = 0; i < 8; i++)
  {
    const unsigned char *end;
    unsigned long a = strtoul(address, (char**)&end, 16);
    if (a > 65535)
      return 1;
    ip_address[i] = ETH_SwapShort(a);
    if (*end == 0)
      break;
    if (*end != ':')
      return 2;
    address = end + 1;
  }
  if (i != 7)
    return 3;
  memcpy(result, ip_address, 16);
  return 0;
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
  if (memcmp(current_ip, prefix, prefix_length))
  {
    memcpy(&eth_instance.router_mac_address, router_mac, 6);
    memcpy(current_ip, prefix, prefix_length);
    if (eth_instance.debug)
      print_ipv6_raw("My IP", current_ip);
  }
  eth_set_prefix_callback();
}

void print_ipv6_raw(const char *title, const unsigned char *ip)
{
 eth_instance.printf_func("%s: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
         title,
         ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7],
         ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
}
