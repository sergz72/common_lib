#ifndef _SCD4X_H
#define _SCD4X_H

#define SCD4X_SENSOR_ADDR 0x62

typedef struct
{
  unsigned short temperature;
  unsigned short humidity;
  unsigned int co2;
} scd4x_result;

#ifdef __cplusplus
extern "C" {
#endif

int scd4x_command(const unsigned char *wdata, unsigned int wlen, unsigned char *rdata, unsigned int rlen); // should be defined in hal.c
int scd4x_write(const unsigned char *data, unsigned int len, bool no_ack_expected); // should be defined in hal.c
int scd4x_read(unsigned char *data, unsigned int len); // should be defined in hal.c

int scd4x_read_measurement (scd4x_result *result);
int scd4x_start_measurement(void);
int scd4x_power_down(void);
int scd4x_wake_up(void);
int scd_get(scd4x_result *result);

#ifdef __cplusplus
}
#endif

#endif
