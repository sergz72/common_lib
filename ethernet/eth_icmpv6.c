#include <eth.h>
#include <eth_icmpv6.h>
#include <eth_queue.h>
#include <stdio.h>
#include <memory.h>

typedef struct
{
  unsigned char option_code;
  unsigned char length;      /* Length in units of 8 bytes) */
} RouterAnnouncementOptionHeader;

typedef struct __attribute__((__packed__))
{
  unsigned char hop_limit;
  unsigned char flags;
  unsigned short router_lifetime;
  unsigned int reachable_time;
  unsigned int retrans_timer;
  RouterAnnouncementOptionHeader option_header;
} RouterAnnouncementPacket;

#define ROUTER_ANNOUNCEMENT_PREFIX_INFO 3
typedef struct __attribute__((__packed__))
{
  RouterAnnouncementOptionHeader option_header;
  unsigned char prefix_length;    /* Prefix length in bits (e.g., 64) */
  unsigned char flags;            /* On-link (L), Autonomous (A), etc. */
  unsigned int  valid_lifetime;   /* Valid lifetime in seconds */
  unsigned int  pref_lifetime;    /* Preferred lifetime in seconds */
  unsigned int  reserved;         /* Reserved / padding field */
  unsigned char prefix[16];       /* The IPv6 network prefix */
} RouterAnnouncementPrefixInfo;

#define ROUTER_ANNOUNCEMENT_ROUTE_INFO 24
typedef struct __attribute__((packed))
{
  RouterAnnouncementOptionHeader option_header;
  unsigned char  prefix_length;    /* Route prefix length in bits */
  unsigned char  flags;            /* Route Preference, etc. */
  unsigned int   route_lifetime;   /* Route lifetime in seconds */
  unsigned char  prefix[16];       /* Destination network prefix */
} RouterAnnouncementRouteInfo;

#define ROUTER_ANNOUNCEMENT_DNS_INFO 25
typedef struct __attribute__((packed))
{
  RouterAnnouncementOptionHeader option_header;
  unsigned short reserved;       /* Reserved / padding field */
  unsigned int   lifetime;       /* DNS server lifetime in seconds */
  ETH_IPV6_Address dns_server;   /* IPv6 address of the DNS server */
} RouterAnnouncementDnsInfo;

#define ROUTER_ANNOUNCEMENT_MTU 5
typedef struct __attribute__((packed))
{
  RouterAnnouncementOptionHeader option_header;
  unsigned short reserved;         /* Reserved / padding field */
  unsigned int   mtu;              /* Link MTU size in bytes */
} RouterAnnouncementMtu;

#define ROUTER_ANNOUNCEMENT_SLLA 1
typedef struct __attribute__((packed))
{
  RouterAnnouncementOptionHeader option_header;
  unsigned char  mac[6];           /* Router Link-Layer MAC address */
} RouterAnnouncementSlla;

static void parse_ra(const RouterAnnouncementOptionHeader *oh, int length, const unsigned char *source_mac)
{
  while (length > 0)
  {
    //eth_instance.printf_func("RouterAnnouncementOptionHeader code %d length %d\n", oh->option_code, oh->length);
    switch (oh->option_code)
    {
      case ROUTER_ANNOUNCEMENT_PREFIX_INFO:
        const RouterAnnouncementPrefixInfo *pi =(const RouterAnnouncementPrefixInfo*)oh;
        ETH_Set_Prefix(pi->prefix, pi->prefix_length, source_mac);
        return;
      default:
        break;
    }
    int l = oh->length * 8;
    length -= l;
    oh = (const RouterAnnouncementOptionHeader *)((const unsigned char*)oh + l);
  }
}

void ETH_ICMPV6_MulticastHandler(const ETH_ICMPV6_FullHeader *icmp_hdr, unsigned int length)
{
  //eth_instance.printf_func("Got a multicast packet with type %02X\n", icmp_hdr->icmpv6_hdr.type);
  switch (icmp_hdr->icmpv6_hdr.type)
  {
    case ETH_ICMPV6_TYPE_RA:
      const RouterAnnouncementPacket *ra = (const RouterAnnouncementPacket *)((const unsigned char*)icmp_hdr + sizeof(ETH_ICMPV6_FullHeader));
      parse_ra(&ra->option_header,
               (int)length - sizeof(ETH_ICMPV6_FullHeader) - sizeof(RouterAnnouncementPacket) + sizeof(RouterAnnouncementOptionHeader),
               icmp_hdr->eth_hdr.src_addr);
      break;
    default:
      break;
  }
}

void ETH_ICMPV6_Handler(const ETH_ICMPV6_FullHeader *icmp_hdr, unsigned int length)
{
  if (icmp_hdr->icmpv6_hdr.type == ETH_ICMPV6_TYPE_ECHO_REQUEST && icmp_hdr->icmpv6_hdr.code == 0) // ICMPV6 echo request
  {
    ETH_ICMPV6_FullHeader *header = (ETH_ICMPV6_FullHeader *)queue_peekTx(&eth_irq_queue);

    for (unsigned int i = 0; i < ETH_HWADDR_LEN; i++)
    {
      header->eth_hdr.src_addr[i] = eth_instance.mac_address[i];
      header->eth_hdr.dest_addr[i] = icmp_hdr->eth_hdr.src_addr[i];
    }
    header->eth_hdr.type = ETH_PROTOCOL_IPV6;
    header->ipv6_hdr.nextHeader = ETH_IPV6_NEXT_HEADER_UDP;
    header->ipv6_hdr.version = 6;
    header->ipv6_hdr.payloadLength = ETH_SwapShort(ETH_ICMPV6_HEADER_LENGTH);
    header->ipv6_hdr.trafficClass = 0;
    header->ipv6_hdr.flowLabel = 0;
    header->ipv6_hdr.hopLimit = 255;
    memcpy(&header->ipv6_hdr.destIP, &icmp_hdr->ipv6_hdr.destIP, sizeof(ETH_IPV6_Address));
    memcpy(&header->ipv6_hdr.sourceIP, &eth_instance.ipv6_address, sizeof(ETH_IPV6_Address));
    header->icmpv6_hdr.type = ETH_ICMPV6_TYPE_ECHO_REPLY;
    header->icmpv6_hdr.code = 0;
    header->icmpv6_hdr.checksum = 0;

    queue_add(&eth_irq_queue, sizeof(ETH_ICMPV6_FullHeader));
  }
}
