#include "tests.h"
#include <stdlib.h>
#include <eth.h>
#include <eth_udp.h>
#include <stdio.h>

unsigned short data[1024];

void parse_packet(const unsigned char *packet, unsigned int *len)
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

const unsigned int checksum_pos = (14 + 40 + 6)/2;

int run_tests(void)
{
  unsigned int l;
  parse_packet(packet5, &l);
  unsigned short checksum = data[checksum_pos];
  data[checksum_pos] = 0;
  unsigned int calculated = ETH_UDP_ComputeChecksum((const ETH_UDP_FullHeader *)data);
  printf("Expected checksum: %04x, calculated checksum %04x\n", checksum, calculated);
  return checksum == calculated;
}
