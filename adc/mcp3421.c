#include <mcp3421.h>

int mcp3421SetConfig(int channel, unsigned char address, const MCP3421Config *cfg)
{
  unsigned char c = cfg->gain|cfg->sample_rate;
  if (cfg->continuous_conversion)
    c |= 0x10;
  if (cfg->start_conversion)
    c |= 0x80;
  return mcp3421Write(channel, address, c);
}

int mcp3421Get18BitVoltage(int channel, unsigned char address, long *voltage)
{
  unsigned char data[3];
  long v;
  int rc = mcp3421Read(channel, address, data, 3);
  if (!rc)
  {
    v = ((long)data[0] << 16) | ((long)data[1] << 8) | (long)data[2];
    if (v & 0x20000)
      v |= 0xFFFC0000;
    *voltage = v;
  }
  return rc;
}

int mcp3421Get16BitVoltage(int channel, unsigned char address, short *voltage)
{
  unsigned char data[2];
  short v;
  int rc = mcp3421Read(channel, address, data, 2);
  if (!rc)
  {
    v = (data[0] << 8) | data[1];
    *voltage = v;
  }
  return rc;
}
