#include "board.h"
#include <sht4x.h>
#include <crc8.h>

#define SHT4x_RESET_CMD                0x94
#define SHT4x_RAW_DATA_SIZE            6

#define MEASUREMENT_DURATION           10 // ms
#define MEASURE_CMD                    0xFD

typedef unsigned char sht4x_raw_data_t [SHT4x_RAW_DATA_SIZE];

static int sht4x_send_command(unsigned char cmd)
{
  return sht40_register_write(&cmd, 1);
}

static int sht4x_reset(void)
{
  int rc = sht4x_send_command(SHT4x_RESET_CMD);
  if (rc)
    return rc;

  delayms(2);

  return 0;
}

static int sht4x_get_raw_data(sht4x_raw_data_t raw_data)
{
  // read raw data
  int rc = sht40_read(raw_data, sizeof(sht4x_raw_data_t));
  if (rc)
    return rc;

  // check temperature crc
  if (crc8(raw_data,2) != raw_data[2])
    return 100;

  // check humidity crc
  if (crc8(raw_data+3,2) != raw_data[5])
    return 101;

  return 0;
}

static void sht4x_compute_values (const sht4x_raw_data_t raw_data, short* temperature, unsigned short* humidity)
{
  *temperature = (short)((((((int)raw_data[0]) << 8) + raw_data[1]) * 17500) / 65535 - 4500);
  *humidity = (unsigned short)((((((unsigned int)raw_data[3]) << 8) + raw_data[4]) * 12500) / 65535 - 600);
}

int sht4x_measure (short* temperature, unsigned short* humidity)
{
  if (!temperature || !humidity) return 1;

  int rc = sht4x_send_command(MEASURE_CMD);
  if (rc)
    return rc;

  delayms(MEASUREMENT_DURATION);

  sht4x_raw_data_t raw_data;

  rc = sht4x_get_raw_data (raw_data);
  if (rc)
    return rc;

  sht4x_compute_values (raw_data, temperature, humidity);

  return 0;
}

int sht4x_init_sensor(void)
{
  return sht4x_reset();
}
