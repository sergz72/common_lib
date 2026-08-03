#ifndef _ETH_H
#define _ETH_H

#define ETH_HWADDR_LEN 6

#define ETH_PROTOCOL_ARP  0x0806
#define ETH_PROTOCOL_IPV4 0x0800
#define ETH_PROTOCOL_IPV6 0xDD86

#ifndef ETH_MAX_PACKET_SIZE
#define ETH_MAX_PACKET_SIZE 1518
#endif

#define PORT_ECHO 7
#define PORT_ECHO_REVERSED 0x0700

typedef struct __attribute__((__packed__))
{
  unsigned char dest_addr[ETH_HWADDR_LEN];
  unsigned char src_addr[ETH_HWADDR_LEN];
  unsigned short type;
} ETH_Header;

typedef enum
{
  ETH_LOGLEVEL_NONE,
  ETH_LOGLEVEL_ERROR,
  ETH_LOGLEVEL_WARINIG,
  ETH_LOGLEVEL_INFO,
  ETH_LOGLEVEL_DEBUG,
  ETH_LOGLEVEL_TRACE
} ETH_LogLevel;

typedef struct
{
  unsigned char mac_address[6];
  unsigned char router_mac_address[6];
  unsigned char global_ipv6_address[16];
  unsigned char local_ipv6_address[16];
  unsigned char ntp_server_address[16];
  int (*puts_func)(const char *str);
  ETH_LogLevel log_level;
  unsigned short flowLabel;
} ETH_Instance;

#define ETH_HEADER_LENGTH 14

void ethernet_packet_received(const void *buffer, unsigned int length);
int ethernet_packet_send(const void *buff, unsigned int length);
void print_ipv6_raw(const char *title, const unsigned char *ip);
void ETH_Set_Prefix(const unsigned char *prefix, unsigned char prefix_length, const unsigned char *router_mac);
void ETH_Common_Init(const unsigned char *mac, const unsigned char *ntp_server_address, int puts_func (const char * str), ETH_LogLevel log_level);
void eth_set_prefix_callback(void);
int ETH_Parse_IPV6(const unsigned char *address, unsigned char *result);
unsigned int calculate_sum(const unsigned short *addr, unsigned int length);
bool ETH_IP_Match(const unsigned char *ip);
const unsigned char *ETH_GetMyIP(const unsigned char *destIP);

static inline unsigned short ETH_SwapShort(const unsigned short v)
{
  return __builtin_bswap16(v);
}

static inline unsigned int ETH_SwapInt(const unsigned int v)
{
  return __builtin_bswap32(v);
}

static inline bool ETH_IsLocalIP(const unsigned char *ip)
{
  return ip[0] == 0xFD;
}

static inline bool ETH_IsGlobalIP(const unsigned char *ip)
{
  return ip[0] < 0xF0;
}

extern ETH_Instance eth_instance;
extern const unsigned char zero_ipv6_address[16];

#endif
