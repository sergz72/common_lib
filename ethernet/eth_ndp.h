#ifndef _ETH_NDP_H
#define _ETH_NDP_H

void ETH_InitNdpTable(void);
void ETH_UpdateNdpTable(const unsigned char *ip_address, const unsigned char *hwaddr);
const unsigned char *ETH_GetHWAddr(const unsigned char *ip_address);

#endif
