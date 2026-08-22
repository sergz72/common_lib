#include <base64.h>
#include <ctype.h>
#include <string.h>

static const unsigned char *base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static int is_base64(unsigned char c)
{
  return isalnum(c) || (c == '+') || (c == '/');
}

static unsigned char base64_chars_find(unsigned char c)
{
  int idx = 0;
  const unsigned char *s = base64_chars;

  while (*s != c)
  {
    idx++;
    s++;
  }

  return idx;
}

unsigned int base64decode(const char *encoded_string, unsigned int in_len, unsigned char *buffer)
{
  int i = 0;
  int j = 0;
  int in_ = 0;
  int out_len = 0;
  unsigned char char_array_4[4], char_array_3[3];

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
  {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i == 4)
    {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars_find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; i < 3; i++)
      {
        *buffer++ = char_array_3[i];
        out_len++;
      }
      i = 0;
    }
  }

  if (i)
  {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars_find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; j < i - 1; j++)
    {
      *buffer++ = char_array_3[j];
      out_len++;
    }
  }

  return out_len;
}

unsigned int base64encode(const unsigned char *bytes_to_encode, unsigned int in_len, unsigned char *ret)
{
	unsigned int out_len, i = 0, j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	out_len = 0;

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; i <4; i++)
			{
				*ret++ = base64_chars[char_array_4[i]];
				out_len++;
			}
			i = 0;
		}
	}

	if (i)
    {
		for(j = i; j < 3; j++)
			char_array_3[j] = 0;

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; j < i + 1; j++)
		{
			*ret++ = base64_chars[char_array_4[j]];
			out_len++;
		}

		while(i++ < 3)
		{
			*ret++ = '=';
			out_len++;
		}
	}

	return out_len;
}
