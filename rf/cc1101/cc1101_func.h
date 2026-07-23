#ifndef _CC1101_FUNC_H
#define _CC1101_FUNC_H

int cc1101Init(unsigned char device_address, unsigned char packet_length, unsigned int frequency, unsigned char rf_mode,
               unsigned char tx_power);
int cc1101Receive(unsigned char **buffer, unsigned char *size);
int cc1101Send(unsigned char host_address, unsigned char *data, unsigned char length);
int cc1101ReceiveStart(void);
int cc1101ReceiveStop(void);
int cc1101PowerDown(void);
int cc1101XOff(void);
bool cc1101ValidatePower(unsigned char tx_power);
bool cc1101ValidateFrequency(unsigned int frequency);
bool cc1101ValidateMode(unsigned char rf_mode);

extern volatile int packet_received;

#endif
