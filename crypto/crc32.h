#ifndef _CRC32_H
#define _CRC32_H

void crc32_init(void);
void crc32_start(void);
void crc32_add(const void *data, unsigned int length);
unsigned int crc32_end(void);

#endif
