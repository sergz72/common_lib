#include "board.h"
#include <eth_queue.h>
#include <eth.h>
#include <queue.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef ETH_DEBUG_QUEUE_SIZE
#define ETH_DEBUG_QUEUE_SIZE 32
#endif

#ifndef ETH_DEBUG_QUEUE_ITEM_LENGTH
#define ETH_DEBUG_QUEUE_ITEM_LENGTH 200
#endif

Queue eth_irq_queue;
static unsigned char eth_irq_buffer[ETH_IRQ_QUEUE_SIZE * ETH_MAX_PACKET_SIZE];
unsigned int eth_irq_message_sizes[ETH_IRQ_QUEUE_SIZE];
Queue eth_user_queue;
static unsigned char eth_user_buffer[ETH_USER_QUEUE_SIZE * ETH_MAX_PACKET_SIZE];
unsigned int eth_user_message_sizes[ETH_USER_QUEUE_SIZE];
FixedQueue eth_debug_queue;
static char eth_debug_buffer[ETH_DEBUG_QUEUE_SIZE*ETH_DEBUG_QUEUE_ITEM_LENGTH];

void ETH_QueueInit(void)
{
  queue_init(&eth_irq_queue, ETH_IRQ_QUEUE_SIZE, ETH_MAX_PACKET_SIZE, eth_irq_buffer, eth_irq_message_sizes);
  queue_init(&eth_user_queue, ETH_USER_QUEUE_SIZE, ETH_MAX_PACKET_SIZE, eth_user_buffer, eth_user_message_sizes);
  fixed_queue_init(&eth_debug_queue, ETH_DEBUG_QUEUE_SIZE, ETH_DEBUG_QUEUE_ITEM_LENGTH, eth_debug_buffer);
}

static unsigned char *ETH_IRQ_QueuePoll(unsigned int *message_size)
{
  return queue_poll(&eth_irq_queue, message_size);
}

static unsigned char *ETH_User_QueuePoll(unsigned int *message_size)
{
  return queue_poll(&eth_user_queue, message_size);
}

static char *ETH_Debug_QueuePoll(void)
{
  unsigned char *v = fixed_queue_peek(&eth_debug_queue);
  if (v)
    fixed_queue_pop(&eth_debug_queue);
  return (char*)v;
}

void ETH_Handler(void)
{
  unsigned int message_size;
  const ETH_Header *buffer;
  while (buffer = (const ETH_Header*)ETH_IRQ_QueuePoll(&message_size))
    ethernet_packet_send(buffer, message_size);
  while (buffer = (const ETH_Header*)ETH_User_QueuePoll(&message_size))
    ethernet_packet_send(buffer, message_size);
  const char *str;
  while (str = ETH_Debug_QueuePoll())
    eth_instance.puts_func(str);
}

void ETH_Puts(const char *str)
{
  fixed_queue_push(&eth_debug_queue, str);
}

void ETH_Printf(const char *format, ...)
{
  va_list vArgs;
  char *p = fixed_queue_get_current_buffer_pointer(&eth_debug_queue);
  va_start(vArgs, format);
  vsnprintf(p, ETH_DEBUG_QUEUE_ITEM_LENGTH, format, vArgs);
  va_end(vArgs);
  fixed_queue_move_next(&eth_debug_queue);
}

static char get_symbol (unsigned char c)
{
  if (c > 9)
    return 'A' + c - 10;
  return '0' + c;
}

void ETH_PrintBuffer(const unsigned char *buffer, unsigned int length)
{
  while (length)
  {
    char *p = fixed_queue_get_current_buffer_pointer(&eth_debug_queue);
    unsigned int l = length > (ETH_DEBUG_QUEUE_ITEM_LENGTH-1) / 3 ? (ETH_DEBUG_QUEUE_ITEM_LENGTH-1) / 3 : length;
    length -= l;
    while (l--)
    {
      unsigned char v = *buffer++;
      *p++ = get_symbol(v >> 4);
      *p++ = get_symbol(v & 0x0F);;
      *p++ = ' ';
    }
    *p = 0;
    fixed_queue_move_next(&eth_debug_queue);
  }
}
