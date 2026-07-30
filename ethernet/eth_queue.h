#ifndef _ETH_QUEUE_H
#define _ETH_QUEUE_H

#include <queue.h>

void ETH_QueueInit(void);
void ETH_Handler(void);

extern Queue eth_irq_queue;
extern Queue eth_user_queue;

#endif
