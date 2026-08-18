#ifndef _BASE64_H
#define _BASE64_H

unsigned int base64decode(const char *encoded_string, unsigned int in_len, unsigned char *buffer);
unsigned int base64encode(const unsigned char *bytes_to_encode, unsigned int in_len, unsigned char *ret);

#endif
