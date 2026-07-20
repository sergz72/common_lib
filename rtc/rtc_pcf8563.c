#include <board.h>
#include <rtc_pcf8563.h>

static unsigned char status2_register;

int pcf8563_init(unsigned char clkout)
{
  unsigned char data[2];

  status2_register = 0;

  data[0] = PCF8563_REG_CLKOUT;
  data[1] = clkout;

  /* write register address, control register */
  /* enable sqw output */
  int rc = i2c_pcf8563_write(data, 2);
  if (rc != 0)
    return rc;

  data[0] = PCF8563_REG_CONTROL_STATUS2;
  data[1] = 0;

  /* write register address, control register */
  /* enable sqw output */
  return i2c_pcf8563_write(data, 2);
}

int pcf8563_get_status(unsigned char *status)
{
  unsigned char reg;

  reg = PCF8563_REG_CONTROL_STATUS1;

  return i2c_pcf8563_transfer(&reg, 1, status, 1);
}

int pcf8563_get_status2(unsigned char *status)
{
  unsigned char reg;

  reg = PCF8563_REG_CONTROL_STATUS2;

  return i2c_pcf8563_transfer(&reg, 1, status, 1);
}

int pcf8563_stop(void)
{
  unsigned char data[2];

  status2_register = 0;

  data[0] = PCF8563_REG_CONTROL_STATUS1;
  data[1] = 0x20;

  return i2c_pcf8563_write(data, 2);
}

int pcf8563_start(void)
{
  unsigned char data[2];

  status2_register = 0;

  data[0] = PCF8563_REG_CONTROL_STATUS1;
  data[1] = 0;

  return i2c_pcf8563_write(data, 2);
}

static unsigned char pack(unsigned char data)
{
  return (data & 0xF) + (data >> 4) * 10;
}

static void pack_alarm(unsigned char data, alarm_item *item)
{
  item->enabled = (data & 0x80) == 0;
  item->value = item->enabled ? (data & 0xF) + (data >> 4) * 10 : 0;
}

static unsigned char unpack(unsigned char data)
{
  return (data % 10) + ((data / 10) << 4);
}

int pcf8563_get(rtc_data *data)
{
  unsigned char data2[7];

  data2[0] = PCF8563_REG_SECONDS;
  int rc = i2c_pcf8563_transfer(data2, 1, data2, 7);
  if (rc != 0)
    return rc;
  data->seconds = pack(data2[0] & 0x7F);
  data->minutes = pack(data2[1] & 0x7F);
  data->hours   = pack(data2[2] & 0x3F);
  data->day     = pack(data2[3] & 0x3F);
  data->wday    = data2[4] & 7;
  data->month   = pack(data2[5] & 0x1F);
  data->year    = 2000 + pack(data2[6]);
  data->yday    = rtc_get_yday(data->year, data->month, data->day);
  return 0;
}

int pcf8563_get_alarm(pcf8563_alarm *adata)
{
  unsigned char data2[7];

  data2[0] = PCF8563_REG_ALARM_MINUTES;
  int rc = i2c_pcf8563_transfer(data2, 1, data2, 4);
  if (rc != 0)
    return rc;
  pack_alarm(data2[0], &adata->minute);
  pack_alarm(data2[1], &adata->hour);
  pack_alarm(data2[2], &adata->day);
  pack_alarm(data2[3], &adata->wday);
  return 0;
}

int pcf8563_set_timer(unsigned char value, unsigned char timer_clock)
{
  unsigned char data[3];

  data[0] = PCF8563_REG_TIMER_CONTROL;
  data[1] = timer_clock | 0x80;
  data[2] = value;
  int rc = i2c_pcf8563_write(data, 3);
  if (rc != 0)
    return rc;
  status2_register |= 1;
  data[0] = PCF8563_REG_CONTROL_STATUS2;
  data[1] = status2_register | 8;
  return i2c_pcf8563_write(data, 2);
}

int pcf8563_cancel_timer(void)
{
  unsigned char data[2];
  data[0] = PCF8563_REG_TIMER_CONTROL;
  data[1] = 0;
  int rc = i2c_pcf8563_write(data, 2);
  if (rc != 0)
    return rc;
  status2_register &= ~1;
  data[0] = PCF8563_REG_CONTROL_STATUS2;
  data[1] = status2_register | 8;
  return i2c_pcf8563_write(data, 2);
}

int pcf8563_set_datetime(unsigned int year, unsigned char month, unsigned char day, unsigned char hour, unsigned char minute, unsigned char seconds)
{
  unsigned char data[8];

  int rc = pcf8563_stop();
  if (rc)
    return rc;

  data[0] = PCF8563_REG_SECONDS;
  data[1] = unpack(seconds);
  data[2] = unpack(minute);
  data[3] = unpack(hour);
  data[4] = unpack(day);
  data[5] = rtc_get_wday(year, month, day);
  data[6] = unpack(month);
  data[7] = unpack(year - 2000);

  rc = i2c_pcf8563_write(data, 8);
  if (rc)
    return rc;

  return pcf8563_start();
}

int pcf8563_set_alarm(const pcf8563_alarm *adata)
{
  unsigned char data[5];

  data[0] = PCF8563_REG_ALARM_MINUTES;
  data[1] = adata->minute.enabled ? unpack(adata->minute.value) : 0x80;
  data[2] = adata->hour.enabled ? unpack(adata->hour.value) : 0x80;
  data[3] = adata->day.enabled ? unpack(adata->day.value) : 0x80;
  data[4] = adata->wday.enabled ? adata->wday.value : 0x80;

  int rc = i2c_pcf8563_write(data, 5);
  if (rc != 0)
    return rc;
  status2_register |= 2;
  data[0] = PCF8563_REG_CONTROL_STATUS2;
  data[1] = status2_register | 4;
  return i2c_pcf8563_write(data, 2);
}

int pcf8563_cancel_alarm(void)
{
  unsigned char data[2];
  status2_register &= ~2;
  data[0] = PCF8563_REG_CONTROL_STATUS2;
  data[1] = status2_register | 4;
  return i2c_pcf8563_write(data, 2);
}
