#include <stdio.h>
#include <rtc_func.h>

int main(void)
{
  rtc_data data;
  data.year = 2099;
  data.month = 5;
  data.day = 8;
  data.hours = 21;
  data.minutes = 30;
  data.seconds = 0;
  unsigned int timestamp = rtc_to_binary(&data);
  printf("%u\n", timestamp);
  return 0;
}
