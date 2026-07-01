#include <mcp47.h>

int mcp47_set_config(int channel, const MCP47Config *cfg)
{
  int rc = mcp47_write_register(channel, MCP47_REGISTER_VREF, cfg->vref);
  if (rc)
    return rc;
  return mcp47_write_register(channel, MCP47_REGISTER_GAIN, cfg->gain);
}

int mcp47_power_down(int channel, unsigned short config)
{
  return mcp47_write_register(channel, MCP47_REGISTER_POWERDOWN, config);
}

int mcp47_set(int channel, unsigned char ch, unsigned short value)
{
  return mcp47_write_register(channel, ch, value);
}
