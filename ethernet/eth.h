#ifndef _ETH_H
#define _ETH_H

#define ETH_HWADDR_LEN 6

#define ETH_PROTOCOL_ARP  0x0806
#define ETH_PROTOCOL_IPV4 0x0800
#define ETH_PROTOCOL_IPV6 0xDD86

#ifndef ETH_MAX_PACKET_SIZE
#define ETH_MAX_PACKET_SIZE 1518
#endif

typedef struct __attribute__((__packed__))
{
  unsigned char dest_addr[ETH_HWADDR_LEN];
  unsigned char src_addr[ETH_HWADDR_LEN];
  unsigned short type;
} ETH_Header;

typedef struct
{
  unsigned long long int address[2];
} ETH_IPV6_Address;

typedef struct
{
  unsigned char mac_address[6];
  unsigned char router_mac_address[6];
  ETH_IPV6_Address ipv6_address;
  int (*printf_func)(const char * format, ...);
  bool debug;
} ETH_Instance;

#define ETH_HEADER_LENGTH 14

void ethernet_packet_received(const void *buffer, unsigned int length);
int ethernet_packet_send(const ETH_Header *buff, unsigned int length);
void print_ipv6_raw(const char *title, const unsigned char *ip);
void ETH_Set_Prefix(const unsigned char *prefix, unsigned char prefix_length, const unsigned char *router_mac);
void ETH_Init(const unsigned char *mac, int printf_func ( const char * format, ... ), bool debug);

static inline bool ETH_IPV6_Compare(const ETH_IPV6_Address *ip1, const ETH_IPV6_Address *ip2)
{
  return ip1->address[0] == ip2->address[0] && ip1->address[1] == ip2->address[1];
}

static inline void ETH_IPV6_Copy(ETH_IPV6_Address *dest, const ETH_IPV6_Address *src)
{
  dest->address[0] = src->address[0];
  dest->address[1] = src->address[1];
}

static inline bool ETH_IPV6_IsZero(const ETH_IPV6_Address *addr)
{
  return addr->address[0] == 0 && addr->address[1] == 0;
}

static inline unsigned short ETH_SwapShort(const unsigned short v)
{
  return __builtin_bswap16(v);
}

extern ETH_Instance eth_instance;

#endif
