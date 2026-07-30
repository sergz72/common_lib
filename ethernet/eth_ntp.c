#include <eth_ntp.h>
#include <eth_udp.h>

bool ntp_time_is_set;

void ETH_NTP_Init(void)
{
  ntp_time_is_set = false;
}

int ETH_NTP_Send_Timestamp_Request(void)
{
  return 1;
}

void ETH_NTP_Process_Timestamp_Reply(const unsigned char *data)
{
  ntp_time_is_set = true;
}
