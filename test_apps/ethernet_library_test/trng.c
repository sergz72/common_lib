#include <trng.h>
#include <sys/random.h>

void trng_generate(unsigned int *data, unsigned int length)
{
  getrandom(data, length * sizeof(int), 0);
}
