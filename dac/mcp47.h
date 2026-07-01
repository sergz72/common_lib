#ifndef _MCP47_H
#define _MCP47_H

#define MCP47_REGISTER_DAC0      0
#define MCP47_REGISTER_DAC1      (1 << 3)
#define MCP47_REGISTER_VREF      (8 << 3)
#define MCP47_REGISTER_POWERDOWN (9 << 3)
#define MCP47_REGISTER_GAIN      (10 << 3)

#define MCP47_VREF_VREF_BUFFERED   3
#define MCP47_VREF_VREF_UNBUFFERED 2
#define MCP47_VREF_BANDGAP         1
#define MCP47_VREF_VDD             0
#define MCP47_VREF(ch, v)          ((v) << (ch << 2))

#define MCP47_POWERDOWN_OPEN    3
#define MCP47_POWERDOWN_100K    2
#define MCP47_POWERDOWN_1K      1
#define MCP47_POWERUP           0
#define MCP47_POWERDOWN(ch, v)   ((v) << (ch << 2))

#define MCP47_GAIN_2 1
#define MCP47_GAIN_1 0
#define MCP47_GAIN(ch, v)        ((v) << (ch + 8))

#define MCP47FVBXXA0_ADDRESS 0x60
#define MCP47FVBXXA1_ADDRESS 0x61
#define MCP47FVBXXA2_ADDRESS 0x62
#define MCP47FVBXXA3_ADDRESS 0x63

typedef struct
{
  unsigned short vref;
  unsigned short gain;
} MCP47Config;

int mcp47_set_config(int channel, const MCP47Config *cfg);
int mcp47_power_down(int channel, unsigned short config);
int mcp47_set(int channel, unsigned char ch, unsigned short value);

int mcp47_write_register(int channel, unsigned char reg, unsigned short value); // should be defined in hal.c

#endif
