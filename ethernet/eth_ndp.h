#ifndef _ETH_NDP_H
#define _ETH_NDP_H

#include <eth.h>

void ETH_InitNdpTable(void);
void ETH_UpdateNdpTable(const ETH_IPV6_Address *ip_address, const unsigned char *hwaddr);
const unsigned char *ETH_GetHWAddr(const ETH_IPV6_Address *ip_address);

#endif
