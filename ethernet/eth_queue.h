#ifndef _ETH_QUEUE_H
#define _ETH_QUEUE_H

#include <queue.h>
#include <fixed_queue.h>

void ETH_QueueInit(void);
void ETH_Handler(void);
void ETH_Printf(const char *format, ...);
void ETH_Puts(const char *str);
void ETH_PrintBuffer(const unsigned char *buffer, unsigned int length);

extern Queue eth_irq_queue;
extern Queue eth_user_queue;
extern FixedQueue eth_debug_queue;

#endif
