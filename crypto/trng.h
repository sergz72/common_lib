#ifndef _TRNG_H
#define _TRNG_H

int trng_init(void);
int trng_generate(unsigned int *data, unsigned int length);

#endif
