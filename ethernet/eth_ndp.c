#include <eth_ndp.h>
#include <string.h>

#ifndef ETH_NDP_TABLE_SIZE
#define ETH_NDP_TABLE_SIZE 16
#endif

typedef struct
{
  ETH_IPV6_Address ip_address;
  unsigned char hwaddr[6];
} ETH_NDP_Table_Item;

static ETH_NDP_Table_Item ndp_table[ETH_NDP_TABLE_SIZE];
static ETH_NDP_Table_Item *ndp_table_p;

void ETH_InitNdpTable(void)
{
  memset(ndp_table, 0, sizeof(ndp_table));
  ndp_table_p = ndp_table;
}

static ETH_NDP_Table_Item *search(const ETH_IPV6_Address *ip_address)
{
  ETH_NDP_Table_Item *p = ndp_table;
  while (p < &ndp_table[ETH_NDP_TABLE_SIZE])
  {
    if (ETH_IPV6_Compare(&p->ip_address, ip_address))
      return p;
    if (ETH_IPV6_IsZero(&p->ip_address))
      return nullptr;
    p++;
  }
  return nullptr;
}

void ETH_UpdateNdpTable(const ETH_IPV6_Address *ip_address, const unsigned char *hwaddr)
{
  ETH_NDP_Table_Item *p = search(ip_address);
  if (p)
    memcpy(p->hwaddr, hwaddr, 6);
  else
  {
    ETH_IPV6_Copy(&ndp_table_p->ip_address, ip_address);
    memcpy(ndp_table_p->hwaddr, hwaddr, 6);
    if (ndp_table_p == &ndp_table[ETH_NDP_TABLE_SIZE-1])
      ndp_table_p = ndp_table;
    else
      ndp_table_p++;
  }
}

const unsigned char *ETH_GetHWAddr(const ETH_IPV6_Address *ip_address)
{
  ETH_NDP_Table_Item *p = search(ip_address);
  if (p)
    return p->hwaddr;
  return eth_instance.router_mac_address;
}