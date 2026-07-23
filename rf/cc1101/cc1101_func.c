#include <board.h>
#include <cc1101.h>
#include <cc1101_func.h>
#include <string.h>

static unsigned char rxdata[CC1101_MAX_PACKET_LENGTH];
static unsigned char txdata[CC1101_MAX_PACKET_LENGTH];

static cc1101_cfg cfgDevice = {
  CC1101_RX_ATTENUATION_0DB,
  0,
  CC1101_PQT0,
  1,
  1,
  CC1101_ADDRESS_CHECK,
  0,
  1,
  CC1101_PACKET_LENGTH_FIXED,
  0,
  0,
  0,
  0,
  0,
  CC1101_MODE_GFSK_600,
  0,
  CC1101_SYNC_MODE_3032,
  0,
  CC1101_PREAMBLE_4,
  0,
  0,
  CC1101_RX_TIME_7,
  CC1101_CCA_MODE_ALWAYS,
  CC1101_RXOFF_MODE,
  CC1101_TXOFF_MODE,
  CC1101_AUTOCAL_FROM_IDLE,
  CC1101_PO_TIMEOUT_150US,
  0,
  0,
  CC1101_TX_POWER_M30_433
};

bool cc1101ValidateMode(unsigned char rf_mode)
{
  return rf_mode <= CC1101_MODE_MAX;
}

bool cc1101ValidateFrequency(unsigned int frequency)
{
  return (frequency >= 430000 && frequency <= 439999) || (frequency >= 863000 && frequency <= 869999);
}

bool cc1101ValidatePower(unsigned char tx_power)
{
  return tx_power <= CC1101_TX_POWER_10_433 && tx_power >= CC1101_TX_POWER_M30_868;
}

int cc1101Init(unsigned char device_address, unsigned char packet_length, unsigned int frequency, unsigned char rf_mode,
               unsigned char tx_power)
{
  unsigned char status;

  if (!cc1101ValidateFrequency(frequency))
    return 1;
  if (packet_length == 0 || packet_length > CC1101_MAX_PACKET_LENGTH)
    return 2;
  if (!cc1101ValidateMode(rf_mode))
    return 3;
  if (!cc1101ValidatePower(tx_power))
    return 4;

  int rc = cc1101_strobe(0, CC1101_STROBE_SRES, &status);
  if (rc)
    return rc;

  rc = cc1101_Check(0);
  if (rc)
    return rc;

  cfgDevice.address = device_address;
  cfgDevice.packetLength = packet_length;
  cfgDevice.freq = frequency;
  cfgDevice.mode = rf_mode;
  cfgDevice.txPower = tx_power;
  rc = cc1101_Init(0, &cfgDevice);
  if (rc)
    return rc;

  return cc1101_strobe(0, CC1101_STROBE_SPWD, &status);
}

int cc1101ReceiveStart(void)
{
  unsigned char status;
  int rc = cc1101_strobe(0, CC1101_STROBE_SFRX, &status);
  if (rc)
    return rc;
  return cc1101_strobe(0, CC1101_STROBE_SRX, &status);
}

int cc1101ReceiveStop(void)
{
  unsigned char status;
  return cc1101_strobe(0, CC1101_STROBE_SIDLE, &status);
}

int cc1101PowerDown(void)
{
  unsigned char status;
  return cc1101_strobe(0, CC1101_STROBE_SPWD, &status);
}

int cc1101XOff(void)
{
  unsigned char status;
  return cc1101_strobe(0, CC1101_STROBE_SXOFF, &status);
}

int cc1101Receive(unsigned char **buffer, unsigned char *size)
{
  unsigned char status;

  if (!cc1101_GD0)
    return 1;

  txdata[0] = CC1101_RXBYTES;
  int rc = cc1101_RW(0, txdata, rxdata, 2);
  if (rc)
  {
    cc1101_strobe(0, CC1101_STROBE_SFRX, &status);
    cc1101_strobe(0, CC1101_STROBE_SIDLE, &status);
    return rc;
  }
  unsigned char sz = rxdata[1];
  *size = sz - 1;

  txdata[0] = CC1101_FIFO | CC1101_READ;
  rc = cc1101_RW(0, txdata, rxdata, sz + 1);
  if (rc)
  {
    cc1101_strobe(0, CC1101_STROBE_SIDLE, &status);
    return rc;
  }

  *buffer = &rxdata[2];

  return 0;
}

int cc1101Send(unsigned char host_address, unsigned char *data, unsigned char length)
{
  unsigned char status;

  int rc = cc1101_strobe(0, CC1101_STROBE_SFTX, &status);
  if (rc)
    return rc;

  memcpy(&txdata[2], data, length);
  txdata[0] = CC1101_FIFO;
  txdata[1] = host_address;

  rc = cc1101_RW(0, txdata, rxdata, length + 2);
  if (rc)
  {
    cc1101_strobe(0, CC1101_STROBE_SIDLE, &status);
    return rc;
  }

  rc = cc1101_strobe(0, CC1101_STROBE_STX, &status);
  if (rc)
  {
    cc1101_strobe(0, CC1101_STROBE_SIDLE, &status);
    return rc;
  }

  return 0;
}
