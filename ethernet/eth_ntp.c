#include <eth_ntp.h>
#include <eth_udp.h>
#include <string.h>

#include "eth_queue.h"

static const ETH_NTP_Packet client_packet =
{
  // LI = 03 (clock unsynchronized), VN = 100 (Version 4), Mode = 011 (Client mode)
  // Binary bit-mask synthesis: 11100011 -> 0xe3
  .li_vn_mode = 0xe3,
  .origTm_f = 0,
  .origTm_s = 0,
  .poll = 0,
  .precision = 0,
  .refId = 0,
  .refTm_f = 0,
  .refTm_s = 0,
  .rootDelay = 0,
  .rootDispersion = 0,
  .rxTm_f = 0,
  .rxTm_s = 0,
  .stratum = 0,
  .txTm_f = 0,
  .txTm_s = NTP_TIMESTAMP_INIT
};

bool ntp_time_is_set;

void ETH_NTP_Init(void)
{
  ntp_time_is_set = false;
}

int ETH_NTP_Send_Timestamp_Request(const unsigned char *address, Queue *q)
{
  return ETH_UDP_Send(NTP_PORT, address, NTP_PORT, &client_packet, sizeof(ETH_NTP_Packet), q);
}

void ETH_NTP_Process_Timestamp_Reply(const void *data)
{
  const ETH_NTP_Packet *reply = data;
  unsigned int tm_s;

  memcpy(&tm_s, &reply->rxTm_s, 4);
  unsigned int unix_time = ETH_SwapInt(tm_s) - NTP_TIMESTAMP_DELTA;
  ntp_time_received_callback(unix_time);
  ntp_time_is_set = true;
}
