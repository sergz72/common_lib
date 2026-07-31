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
  unsigned char mac_address[6];
  unsigned char router_mac_address[6];
  unsigned char ipv6_address[16];
  unsigned char ntp_server_address[16];
  int (*printf_func)(const char * format, ...);
  bool debug;
} ETH_Instance;

#define ETH_HEADER_LENGTH 14

void ethernet_packet_received(const void *buffer, unsigned int length);
int ethernet_packet_send(const ETH_Header *buff, unsigned int length);
void print_ipv6_raw(const char *title, const unsigned char *ip);
void ETH_Set_Prefix(const unsigned char *prefix, unsigned char prefix_length, const unsigned char *router_mac);
void ETH_Init(const unsigned char *mac, const unsigned char *ntp_server_address, int printf_func ( const char * format, ... ), bool debug);
void eth_set_prefix_callback(void);
int ETH_Parse_IPV6(const unsigned char *address, unsigned char *result);

static inline unsigned short ETH_SwapShort(const unsigned short v)
{
  return __builtin_bswap16(v);
}

static inline unsigned int ETH_SwapInt(const unsigned int v)
{
  return __builtin_bswap32(v);
}

extern ETH_Instance eth_instance;
extern const unsigned char zero_ipv6_address[16];

#endif
