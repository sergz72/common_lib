#ifndef _ETH_NTP_H
#define _ETH_NTP_H

void ETH_NTP_Init(void);
int ETH_NTP_Send_Timestamp_Request(void);
void ETH_NTP_Process_Timestamp_Reply(const unsigned char *data);

extern bool ntp_time_is_set;

#endif
