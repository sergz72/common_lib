#ifndef _ETH_NTP_H
#define _ETH_NTP_H

#define NTP_PORT 123 // 0x7B
#define NTP_PORT_REVERSED 0x7B00
#define NTP_TIMESTAMP_DELTA 2208988800 // Seconds difference between 1900 and 1970 (0x83AA7E80)
#define NTP_TIMESTAMP_DELTA_REVERSED 0x807EAA83

// Structured layout matching the 48-byte NTP Header Specification
typedef struct {
  unsigned char li_vn_mode;    // LI (2 bits), VN (3 bits), Mode (3 bits)
  unsigned char stratum;       // Stratum level of the local clock
  unsigned char poll;          // Maximum interval between successive messages
  unsigned char precision;     // Precision of the local clock
  unsigned int rootDelay;      // Total round trip delay to the reference clock
  unsigned int rootDispersion; // Total dispersion to the reference clock
  unsigned int refId;          // Reference clock identifier
  unsigned int refTm_s;        // Reference timestamp (seconds)
  unsigned int refTm_f;        // Reference timestamp (fraction)
  unsigned int origTm_s;       // Originate timestamp (seconds)
  unsigned int origTm_f;       // Originate timestamp (fraction)
  unsigned int rxTm_s;         // Receive timestamp (seconds)
  unsigned int rxTm_f;         // Receive timestamp (fraction)
  unsigned int txTm_s;         // Transmit timestamp (seconds) - Used for parsing
  unsigned int txTm_f;         // Transmit timestamp (fraction)
} ETH_NTP_Packet;

void ETH_NTP_Init(void);
int ETH_NTP_Send_Timestamp_Request(void);
void ETH_NTP_Process_Timestamp_Reply(const void *data);
void ntp_time_received_callback(unsigned int unix_time);

extern bool ntp_time_is_set;

#endif
