#include <eth.h>
#include <eth_icmpv6.h>
#include <eth_queue.h>
#include <eth_ndp.h>
#include <stdio.h>
#include <memory.h>

#define NA_FLAG_ROUTER    0x80//(1U << 31)
#define NA_FLAG_SOLICITED 0x40//(1U << 30)
#define NA_FLAG_OVERRIDE  0x20//(1U << 29)

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
  unsigned char dns_server[16];   /* IPv6 address of the DNS server */
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

typedef struct
{
  unsigned char type;
  unsigned char length;
  unsigned char mac[6];
} ETH_ICMPV6_Address_Option;

typedef struct
{
  unsigned int reserved;    /* Reserved: Set to 0 */
  unsigned char target[16]; /* Target IPv6 Address */
  ETH_ICMPV6_Address_Option ao;
} NeighborSolicitationPacket;

typedef struct
{
  unsigned int flags;
  unsigned char target[16];
  ETH_ICMPV6_Address_Option ao;
} NeighborAdvertisementPacket;

typedef struct __attribute__((__packed__))
{
  ETH_Header eth_hdr;
  ETH_IPV6_Header ipv6_hdr;
  ETH_ICMPV6_Header icmpv6_hdr;
  NeighborSolicitationPacket ns;
} ETH_ICMPV6_NS_FullHeader;

typedef struct __attribute__((__packed__))
{
  ETH_Header eth_hdr;
  ETH_IPV6_Header ipv6_hdr;
  ETH_ICMPV6_Header icmpv6_hdr;
  NeighborAdvertisementPacket na;
} ETH_ICMPV6_NA_FullHeader;

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
        break;
      default:
        break;
    }
    int l = oh->length * 8;
    length -= l;
    oh = (const RouterAnnouncementOptionHeader *)((const unsigned char*)oh + l);
  }
}

unsigned short ETH_ICMPV6_ComputeChecksum(const ETH_ICMPV6_FullHeader *header)
{
  unsigned int sum = calculate_sum((unsigned short*)&header->ipv6_hdr.sourceIP, 32);
  sum += ETH_IPV6_NEXT_HEADER_ICMPV6;
  unsigned int l = ETH_SwapShort(header->ipv6_hdr.payloadLength);
  sum += l;
  sum += calculate_sum((unsigned short*)&header->icmpv6_hdr, l);

  // Fold 32-bit sum to 16 bits
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  unsigned short checksum = (unsigned short)~sum;

  // IPv6 UDP Rule: If checksum is 0, return 0xFFFF
  if (checksum == 0) {
    return 0xFFFF;
  }

  return ETH_SwapShort(checksum);
}

static void parse_ns(const ETH_ICMPV6_NS_FullHeader *ns)
{
  if (eth_instance.log_level >= ETH_LOGLEVEL_DEBUG)
    ETH_Puts("Got a ns packet");
  if (ETH_IP_Match(ns->ns.target))
  {
    if (eth_instance.log_level >= ETH_LOGLEVEL_DEBUG)
      ETH_Puts("Sending na reply...");
    ETH_ICMPV6_NA_FullHeader *header = (ETH_ICMPV6_NA_FullHeader *)queue_peekTx(&eth_irq_queue);
    memcpy(header->eth_hdr.src_addr, eth_instance.mac_address, 6);
    memcpy(header->eth_hdr.dest_addr, ns->eth_hdr.src_addr, 6);
    header->eth_hdr.type = ETH_PROTOCOL_IPV6;
    header->ipv6_hdr.nextHeader = ETH_IPV6_NEXT_HEADER_ICMPV6;
    header->ipv6_hdr.version_trafficClass_flowLabel_high = ns->ipv6_hdr.version_trafficClass_flowLabel_high;
    header->ipv6_hdr.payloadLength = ETH_SwapShort(sizeof(NeighborAdvertisementPacket) + ETH_ICMPV6_HEADER_LENGTH);
    header->ipv6_hdr.flowLabel_low = ns->ipv6_hdr.flowLabel_low;
    header->ipv6_hdr.hopLimit = 255;
    memcpy(header->ipv6_hdr.destIP, ns->ipv6_hdr.sourceIP, 16);
    memcpy(header->ipv6_hdr.sourceIP, ns->ns.target, 16);
    header->icmpv6_hdr.type = ETH_ICMPV6_TYPE_NA;
    header->icmpv6_hdr.code = 0;
    header->icmpv6_hdr.checksum = 0;
    header->na.flags = NA_FLAG_SOLICITED | NA_FLAG_OVERRIDE;
    memcpy(header->na.target, ns->ns.target, 16);
    header->na.ao.type = 2;
    header->na.ao.length = 1;
    memcpy(header->na.ao.mac, eth_instance.mac_address, 6);

    header->icmpv6_hdr.checksum =
      ETH_ICMPV6_ComputeChecksum((const ETH_ICMPV6_FullHeader *)header);
    
    queue_add(&eth_irq_queue, sizeof(ETH_ICMPV6_NA_FullHeader));
  }
}

static void parse_na(const ETH_ICMPV6_NA_FullHeader *na)
{
  if (eth_instance.log_level >= ETH_LOGLEVEL_INFO)
    ETH_Puts("Got a na packet");
  if (na->ipv6_hdr.payloadLength != 0x2000 || na->na.ao.type != 2) // 32
    return;
  if (eth_instance.log_level >= ETH_LOGLEVEL_INFO)
    ETH_Puts("Updating NDP table...");
  ETH_UpdateNdpTable(na->na.target, na->na.ao.mac);
}

void ETH_ICMPV6_MulticastHandler(const ETH_ICMPV6_FullHeader *icmp_hdr, unsigned int length)
{
  if (eth_instance.log_level >= ETH_LOGLEVEL_DEBUG)
    ETH_Printf("Got a multicast packet with type %02X", icmp_hdr->icmpv6_hdr.type);
  switch (icmp_hdr->icmpv6_hdr.type)
  {
    case ETH_ICMPV6_TYPE_RA:
      const RouterAnnouncementPacket *ra = (const RouterAnnouncementPacket *)((const unsigned char*)icmp_hdr + sizeof(ETH_ICMPV6_FullHeader));
      parse_ra(&ra->option_header,
               (int)length - sizeof(ETH_ICMPV6_FullHeader) - sizeof(RouterAnnouncementPacket) + sizeof(RouterAnnouncementOptionHeader),
               icmp_hdr->eth_hdr.src_addr);
      break;
    case ETH_ICMPV6_TYPE_NS:
      parse_ns((const ETH_ICMPV6_NS_FullHeader*)icmp_hdr);
      break;
    default:
      break;
  }
}

void ETH_ICMPV6_Handler(const ETH_ICMPV6_FullHeader *icmp_hdr, unsigned int length)
{
  if (icmp_hdr->icmpv6_hdr.type == ETH_ICMPV6_TYPE_NA)
    parse_na((const ETH_ICMPV6_NA_FullHeader*)icmp_hdr);
  else if (icmp_hdr->icmpv6_hdr.type == ETH_ICMPV6_TYPE_ECHO_REQUEST && icmp_hdr->icmpv6_hdr.code == 0) // ICMPV6 echo request
  {
    unsigned int l = ETH_SwapShort(icmp_hdr->ipv6_hdr.payloadLength);
    if (l < ETH_ICMPV6_HEADER_LENGTH)
      return;

    if (eth_instance.log_level >= ETH_LOGLEVEL_DEBUG)
      ETH_Puts("Got a ICMPV6 echo request");

    ETH_ICMPV6_FullHeader *header = (ETH_ICMPV6_FullHeader *)queue_peekTx(&eth_irq_queue);

    for (unsigned int i = 0; i < ETH_HWADDR_LEN; i++)
    {
      header->eth_hdr.src_addr[i] = eth_instance.mac_address[i];
      header->eth_hdr.dest_addr[i] = icmp_hdr->eth_hdr.src_addr[i];
    }
    header->eth_hdr.type = ETH_PROTOCOL_IPV6;
    header->ipv6_hdr.nextHeader = ETH_IPV6_NEXT_HEADER_ICMPV6;
    header->ipv6_hdr.version_trafficClass_flowLabel_high = icmp_hdr->ipv6_hdr.version_trafficClass_flowLabel_high;
    header->ipv6_hdr.payloadLength = icmp_hdr->ipv6_hdr.payloadLength;
    header->ipv6_hdr.flowLabel_low = icmp_hdr->ipv6_hdr.flowLabel_low;
    header->ipv6_hdr.hopLimit = 255;
    memcpy(header->ipv6_hdr.destIP, icmp_hdr->ipv6_hdr.sourceIP, 16);
    memcpy(header->ipv6_hdr.sourceIP, icmp_hdr->ipv6_hdr.destIP, 16);
    header->icmpv6_hdr.type = ETH_ICMPV6_TYPE_ECHO_REPLY;
    header->icmpv6_hdr.code = 0;
    header->icmpv6_hdr.checksum = 0;

    unsigned int data_length = l - ETH_ICMPV6_HEADER_LENGTH;

    memcpy((unsigned char*)header + sizeof(ETH_ICMPV6_FullHeader), (unsigned char*)icmp_hdr + sizeof(ETH_ICMPV6_FullHeader),
            data_length);

    header->icmpv6_hdr.checksum =
      ETH_ICMPV6_ComputeChecksum(header);

    queue_add(&eth_irq_queue, sizeof(ETH_ICMPV6_FullHeader) + data_length);
  }
}

void ETH_NS_Send(const unsigned char *address)
{
  const unsigned char *myip = ETH_GetMyIP(address);
  ETH_ICMPV6_NS_FullHeader *header = (ETH_ICMPV6_NS_FullHeader *)queue_peekTx(&eth_user_queue);
  memcpy(header->eth_hdr.src_addr, eth_instance.mac_address, 6);
  header->eth_hdr.dest_addr[0] = 0x33;
  header->eth_hdr.dest_addr[1] = 0x33;
  header->eth_hdr.dest_addr[2] = 0xFF;
  header->eth_hdr.dest_addr[3] = address[13];
  header->eth_hdr.dest_addr[4] = address[14];
  header->eth_hdr.dest_addr[5] = address[15];
  header->eth_hdr.type = ETH_PROTOCOL_IPV6;
  header->ipv6_hdr.nextHeader = ETH_IPV6_NEXT_HEADER_ICMPV6;
  header->ipv6_hdr.version_trafficClass_flowLabel_high = 0x60;
  header->ipv6_hdr.payloadLength = ETH_SwapShort(sizeof(NeighborSolicitationPacket) + ETH_ICMPV6_HEADER_LENGTH);
  header->ipv6_hdr.flowLabel_low = eth_instance.flowLabel++;
  header->ipv6_hdr.hopLimit = 255;
  memset(header->ipv6_hdr.destIP, 0, 16);
  header->ipv6_hdr.destIP[0] = 0xFF;
  header->ipv6_hdr.destIP[1] = 2;
  header->ipv6_hdr.destIP[11] = 1;
  header->ipv6_hdr.destIP[12] = 0xFF;
  header->ipv6_hdr.destIP[13] = address[13];
  header->ipv6_hdr.destIP[14] = address[14];
  header->ipv6_hdr.destIP[15] = address[15];
  memcpy(header->ipv6_hdr.sourceIP, myip, 16);
  header->icmpv6_hdr.type = ETH_ICMPV6_TYPE_NS;
  header->icmpv6_hdr.code = 0;
  header->icmpv6_hdr.checksum = 0;
  header->ns.reserved = 0;
  memcpy(header->ns.target, address, 16);
  header->ns.ao.type = 1;
  header->ns.ao.length = 1;
  memcpy(header->ns.ao.mac, eth_instance.mac_address, 6);

  header->icmpv6_hdr.checksum =
    ETH_ICMPV6_ComputeChecksum((const ETH_ICMPV6_FullHeader *)header);

  queue_add(&eth_user_queue, sizeof(ETH_ICMPV6_NS_FullHeader));
}
