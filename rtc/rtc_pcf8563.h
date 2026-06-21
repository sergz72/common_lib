#ifndef _RTC_PCF8563_H
#define _RTC_PCF8563_H

#include <rtc_func.h>
#include <stdbool.h>

#define PCF8563_I2C_ADDRESS 0xA2

#define PCF8563_CLKOUT_DISABLE   0
#define PCF8563_CLKOUT_32768Hz   0x80
#define PCF8563_CLKOUT_1024Hz    0x81
#define PCF8563_CLKOUT_32Hz      0x82
#define PCF8563_CLKOUT_1Hz       0x83

#define PCF8563_TIMER_CLOCK_4096Hz 0
#define PCF8563_TIMER_CLOCK_64Hz   1
#define PCF8563_TIMER_CLOCK_1Hz    2
#define PCF8563_TIMER_CLOCK_1d60Hz 3

#define PCF8563_REG_CONTROL_STATUS1 0
#define PCF8563_REG_CONTROL_STATUS2 1

#define PCF8563_REG_SECONDS         2
#define PCF8563_REG_MINUTES         3
#define PCF8563_REG_HOURS           4
#define PCF8563_REG_DAYS            5
#define PCF8563_REG_WEEKDAYS        6
#define PCF8563_REG_MONTHS          7
#define PCF8563_REG_YEARS           8

#define PCF8563_REG_ALARM_MINUTES   9
#define PCF8563_REG_ALARM_HOURS     10
#define PCF8563_REG_ALARM_DAY       11
#define PCF8563_REG_ALARM_WEEKDAY   12

#define PCF8563_REG_CLKOUT          13
#define PCF8563_REG_TIMER_CONTROL   14
#define PCF8563_REG_TIMER           15

typedef struct
{
  bool enabled;
  unsigned char value;
} alarm_item;

typedef struct
{
  alarm_item minute;
  alarm_item hour;
  alarm_item day;
  alarm_item wday;
} pcf8563_alarm;

int pcf8563_init(unsigned char clkout);
int pcf8563_get_status(unsigned char *status);
int pcf8563_stop(void);
int pcf8563_start(void);
int pcf8563_get(rtc_data *data);
int pcf8563_get_alarm(pcf8563_alarm *data);
int pcf8563_set_timer(unsigned char value, unsigned char timer_clock);
int pcf8563_cancel_timer(void);
int pcf8563_set_datetime(int year, int month, int day, int hour, int minute, int seconds);
int pcf8563_set_alarm(const pcf8563_alarm *adata);
int pcf8563_cancel_alarm(void);

int i2c_pcf8563_write(const unsigned char *data, int data_length); // should be defined in hal.c
int i2c_pcf8563_transfer(const unsigned char *wdata, int wdata_length, unsigned char *rdata, int rdata_length); // should be defined in hal.c

#endif
