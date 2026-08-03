#include "tests.h"
#include <stdlib.h>
#include <eth.h>
#include <eth_udp.h>
#include <eth_icmpv6.h>
#include <stdio.h>

unsigned short data[1024];

void parse_packet2(const unsigned char *packet, unsigned int *len)
{
  unsigned char c;
  const unsigned char *p = nullptr;
  unsigned int l = 0;
  unsigned int total = 0;
  unsigned short *data_p = data;
  while (c = *packet)
  {
    if (c > ' ')
    {
      if (!p)
        p = packet;
      l++;
    }
    else
    {
      if (l == 4)
      {
        *data_p++ = ETH_SwapShort((unsigned short)strtoul(p, nullptr, 16));
        total += 2;
      }
      l = 0;
      p = nullptr;
    }
    packet++;
  }
  if (l == 4)
  {
    *data_p++ = ETH_SwapShort((unsigned short)strtoul(p, nullptr, 16));
    total += 2;
  }
  *len = total;
}

void parse_packet1(const unsigned char *packet, unsigned int *len)
{
  unsigned char c;
  const unsigned char *p = nullptr;
  unsigned int l = 0;
  unsigned int total = 0;
  unsigned char *data_p = (unsigned char *)data;
  while (c = *packet)
  {
    if (c > ' ')
    {
      if (!p)
        p = packet;
      l++;
    }
    else
    {
      if (l == 2)
      {
        *data_p++ = (unsigned char)strtoul(p, nullptr, 16);
        total++;
      }
      l = 0;
      p = nullptr;
    }
    packet++;
  }
  if (l == 2)
  {
    *data_p++ = (unsigned char)strtoul(p, nullptr, 16);
    total++;
  }
  *len = total;
}

const unsigned int checksum_pos = (14 + 40 + 6)/2;

/*int run_tests(void)
{
  unsigned int l;
  parse_packet(packet5, &l);
  unsigned short checksum = data[checksum_pos];
  data[checksum_pos] = 0;
  unsigned int calculated = ETH_UDP_ComputeChecksum((const ETH_UDP_FullHeader *)data);
  printf("Expected checksum: %04x, calculated checksum %04x\n", checksum, calculated);
  return checksum == calculated;
}*/

const unsigned int checksum_pos_icmpv6 = (14 + 40 + 2)/2;

int run_tests(void)
{
  unsigned int l;
  parse_packet1(icmpv6_packet1, &l);
  unsigned short checksum = data[checksum_pos_icmpv6];
  data[checksum_pos_icmpv6] = 0;
  unsigned int calculated = ETH_ICMPV6_ComputeChecksum((const ETH_ICMPV6_FullHeader *)data);
  printf("Expected checksum: %04x, calculated checksum %04x\n", checksum, calculated);
  return checksum == calculated;
}
