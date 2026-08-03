#include <eth.h>
#include <eth_ipv6.h>
#include <string.h>
#include <eth_queue.h>
#include <eth_ndp.h>
#include <trng.h>
#include <eth_ntp.h>
#include <stdlib.h>

const unsigned char zero_ipv6_address[16] = {0};

ETH_Instance eth_instance;

void ETH_Common_Init(const unsigned char *mac, const unsigned char *ntp_server_address, int puts_func (const char * str), ETH_LogLevel log_level)
{
  memset(&eth_instance, 0, sizeof(ETH_Instance));
  if (ntp_server_address)
    memcpy(&eth_instance.ntp_server_address, ntp_server_address, 16);
  eth_instance.log_level = log_level;
  eth_instance.puts_func = puts_func;
  memcpy(eth_instance.mac_address, mac, 6);
  trng_generate((unsigned int*)&eth_instance.global_ipv6_address, 4);
  trng_generate((unsigned int*)&eth_instance.local_ipv6_address, 4);
  eth_instance.global_ipv6_address[0] = 0;
  eth_instance.local_ipv6_address[0] = 0;
  eth_instance.flowLabel = 1;
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

  if (eth_instance.log_level >= ETH_LOGLEVEL_TRACE)
  {
    ETH_Printf("Got a packet %d bytes", length);
    ETH_PrintBuffer(buffer, length);
  }
  switch (eth_hdr->type)
  {
    case ETH_PROTOCOL_IPV6:
      if (eth_hdr->dest_addr[0] == 0x33 && eth_hdr->dest_addr[1] == 0x33)
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
  if (ETH_IsLocalIP(prefix))
  {
    char *current_ip = (char*)&eth_instance.local_ipv6_address;
    if (memcmp(current_ip, prefix, prefix_length))
    {
      memcpy(current_ip, prefix, prefix_length);
      if (eth_instance.log_level >= ETH_LOGLEVEL_INFO)
        print_ipv6_raw("My IP", current_ip);
    }
  }
  else
  {
    char *current_ip = (char*)&eth_instance.global_ipv6_address;
    if (memcmp(current_ip, prefix, prefix_length))
    {
      memcpy(&eth_instance.router_mac_address, router_mac, 6);
      memcpy(current_ip, prefix, prefix_length);
      if (eth_instance.log_level >= ETH_LOGLEVEL_INFO)
        print_ipv6_raw("My IP", current_ip);
    }
    eth_set_prefix_callback();
  }
}

void print_ipv6_raw(const char *title, const unsigned char *ip)
{
  ETH_Printf("%s: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
         title,
         ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7],
         ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
}

unsigned int calculate_sum(const unsigned short *addr, unsigned int length)
{
  unsigned int sum = 0;

  while (length > 1)
  {
    sum += ETH_SwapShort(*addr++);
    length -= 2;
  }

  // Handle an odd byte if present
  if (length != 0)
    sum += *(unsigned char *)addr << 8;

  return sum;
}

bool ETH_IP_Match(const unsigned char *ip)
{
  return !memcmp(ip, eth_instance.global_ipv6_address, 16) || !memcmp(ip, eth_instance.local_ipv6_address, 16);
}

const unsigned char *ETH_GetMyIP(const unsigned char *destIP)
{
  if (ETH_IsLocalIP(destIP) && eth_instance.local_ipv6_address[0] != 0)
    return eth_instance.local_ipv6_address;
  if (ETH_IsGlobalIP(destIP) && eth_instance.global_ipv6_address[0] != 0)
    return eth_instance.global_ipv6_address;
  return nullptr;
}
