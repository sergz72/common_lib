#include <eth_ntp.h>
#include <eth_udp.h>
#include <string.h>

bool ntp_time_is_set;

void ETH_NTP_Init(void)
{
  ntp_time_is_set = false;
}

int ETH_NTP_Send_Timestamp_Request(void)
{
  ETH_NTP_Packet packet;
  memset(&packet, 0, sizeof(ETH_NTP_Packet));

  // LI = 00 (no warning), VN = 100 (Version 4), Mode = 011 (Client mode)
  // Binary bit-mask synthesis: 00100011 -> 0x23
  packet.li_vn_mode = 0x23;
  packet.txTm_s = ETH_SwapInt(NTP_TIMESTAMP_DELTA);
  //packet.txTm_f = 0;

  return ETH_UDP_Send(NTP_PORT, &eth_instance.ntp_server_address, NTP_PORT, &packet, sizeof(ETH_NTP_Packet));
}

void ETH_NTP_Process_Timestamp_Reply(const unsigned char *data)
{
  ntp_time_is_set = true;
}
