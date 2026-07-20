#ifndef _SHT4X_H
#define _SHT4X_H

#ifdef __cplusplus
extern "C" {
#endif

int sht40_register_write(const unsigned char *data, unsigned int len);
int sht40_read(unsigned char *data, unsigned int len);

int sht4x_init_sensor(void);
int sht4x_measure (short* temperature, unsigned short* humidity);

#ifdef __cplusplus
}
#endif

#endif
