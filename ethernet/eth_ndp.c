#include <eth_ndp.h>
#include <string.h>

#include "eth.h"

#ifndef ETH_NDP_TABLE_SIZE
#define ETH_NDP_TABLE_SIZE 16
#endif

typedef struct
{
  unsigned char ip_address[16];
  unsigned char hwaddr[6];
} ETH_NDP_Table_Item;

static ETH_NDP_Table_Item ndp_table[ETH_NDP_TABLE_SIZE];
static ETH_NDP_Table_Item *ndp_table_p;

void ETH_InitNdpTable(void)
{
  memset(ndp_table, 0, sizeof(ndp_table));
  ndp_table_p = ndp_table;
}

static ETH_NDP_Table_Item *search(const unsigned char *ip_address)
{
  ETH_NDP_Table_Item *p = ndp_table;
  while (p < &ndp_table[ETH_NDP_TABLE_SIZE])
  {
    if (!memcmp(p->ip_address, ip_address, 16))
      return p;
    if (!memcmp(p->ip_address, zero_ipv6_address, 16))
      return nullptr;
    p++;
  }
  return nullptr;
}

void ETH_UpdateNdpTable(const unsigned char *ip_address, const unsigned char *hwaddr)
{
  ETH_NDP_Table_Item *p = search(ip_address);
  if (p)
    memcpy(p->hwaddr, hwaddr, 6);
  else
  {
    memcpy(ndp_table_p->ip_address, ip_address, 16);
    memcpy(ndp_table_p->hwaddr, hwaddr, 6);
    if (ndp_table_p == &ndp_table[ETH_NDP_TABLE_SIZE-1])
      ndp_table_p = ndp_table;
    else
      ndp_table_p++;
  }
}

const unsigned char *ETH_GetHWAddr(const unsigned char *ip_address)
{
  ETH_NDP_Table_Item *p = search(ip_address);
  if (p)
    return p->hwaddr;
  return eth_instance.router_mac_address;
}