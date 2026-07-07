#include "board.h"
#include <scd4x.h>
#include <crc8.h>

#define SCD4x_RAW_DATA_SIZE 18

static unsigned char scd4x_raw_data [SCD4x_RAW_DATA_SIZE];

static int validate_raw_data_item(int offset, const char *name)
{
  return crc8(scd4x_raw_data+offset,2) != scd4x_raw_data[offset+2];
}

static int validate_raw_data(void)
{
  int rc = validate_raw_data_item(0, "CO2");
  if (rc)
    return rc;

  rc = validate_raw_data_item(3, "temperature");
  if (rc)
    return rc;

  return validate_raw_data_item(6, "humidity");
}

static void scd4x_compute_values (scd4x_result *result)
{
  unsigned int co2 =
    (((unsigned int)scd4x_raw_data[0]) << 8) |
    (unsigned int)scd4x_raw_data[1];
  unsigned int temperature =
    (((unsigned int)scd4x_raw_data[3]) << 8) |
    (unsigned int)scd4x_raw_data[4];
  unsigned int humidity =
    (((unsigned int)scd4x_raw_data[6]) << 8) |
    (unsigned int)scd4x_raw_data[7];
  result->co2 = co2 * 100;
  result->temperature = (unsigned short)(-4500 + 17500 * temperature / 65535);
  result->humidity = (unsigned short)(10000 * humidity / 65535);
}

int scd4x_start_measurement(void)
{
  unsigned char data[2];
  data[0] = 0x21;
  data[1] = 0x9D;
  return scd4x_write(data, 2, false);
}

int scd4x_power_down(void)
{
  unsigned char data[2];
  data[0] = 0x36;
  data[1] = 0xE0;
  return scd4x_write(data, 2, false);
}

int scd4x_wake_up(void)
{
  unsigned char data[2];
  data[0] = 0x36;
  data[1] = 0xF6;
  int rc = scd4x_write(data, 2, true);
  delayms(30);
  return rc;
}

static int scd4x_get_status(unsigned short *status)
{
  unsigned char data[2];
  data[0] = 0xE4;
  data[1] = 0xB8;
  int rc = scd4x_write(data, 2, false);
  if (rc)
    return rc;
  delayms(2);
  rc = scd4x_read(scd4x_raw_data, 3);
  if (rc)
    return rc;
  rc = validate_raw_data_item(0, "status");
  if (rc)
    return rc;
  *status = ((unsigned short)scd4x_raw_data[0] << 8) | (unsigned short)scd4x_raw_data[1];
  return 0;
}

int scd4x_read_measurement (scd4x_result *result)
{
  unsigned short status;
  for (;;)
  {
    const int rc = scd4x_get_status(&status);
    if (rc)
      return rc;
    if ((status & 0x3FF) != 0)
      break;
    delayms(1000);
  }

  unsigned char data[2];
  data[0] = 0xEC;
  data[1] = 0x05;
  int rc = scd4x_write(data, 2, false);
  if (rc)
    return rc;
  delayms(2);
  rc = scd4x_read(scd4x_raw_data, sizeof(scd4x_raw_data));
  if (rc)
    return rc;
  rc = validate_raw_data();
  if (rc)
    return rc;
  scd4x_compute_values(result);
  return 0;
}

int scd_get(scd4x_result *result)
{
  scd4x_wake_up(); // no acknowledge for this command - no error checking
  int rc = scd4x_start_measurement();
  if (rc)
    return rc;
  delayms(6000);
  rc = scd4x_read_measurement(result);
  if (rc)
    return rc;
  return scd4x_power_down();
}
