#include "board.h"
#include <eth_queue.h>
#include <eth.h>
#include <queue.h>

Queue eth_irq_queue;
static unsigned char eth_irq_buffer[ETH_IRQ_QUEUE_SIZE * ETH_MAX_PACKET_SIZE];
unsigned int eth_irq_message_sizes[ETH_IRQ_QUEUE_SIZE];
Queue eth_user_queue;
static unsigned char eth_user_buffer[ETH_USER_QUEUE_SIZE * ETH_MAX_PACKET_SIZE];
unsigned int eth_user_message_sizes[ETH_USER_QUEUE_SIZE];

void ETH_QueueInit(void)
{
  queue_init(&eth_irq_queue, ETH_IRQ_QUEUE_SIZE, ETH_MAX_PACKET_SIZE, eth_irq_buffer, eth_irq_message_sizes);
  queue_init(&eth_user_queue, ETH_USER_QUEUE_SIZE, ETH_MAX_PACKET_SIZE, eth_user_buffer, eth_user_message_sizes);
}

static unsigned char *ETH_IRQ_QueuePoll(unsigned int *message_size)
{
  return queue_poll(&eth_irq_queue, message_size);
}

static unsigned char *ETH_User_QueuePoll(unsigned int *message_size)
{
  return queue_poll(&eth_user_queue, message_size);
}

void ETH_Handler(void)
{
  unsigned int message_size;
  const ETH_Header *buffer;
  while (buffer = (const ETH_Header*)ETH_IRQ_QueuePoll(&message_size))
    ethernet_packet_send(buffer, message_size);
  while (buffer = (const ETH_Header*)ETH_User_QueuePoll(&message_size))
    ethernet_packet_send(buffer, message_size);
}