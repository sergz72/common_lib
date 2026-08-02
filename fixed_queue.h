#ifndef MAIN_FIXED_QUEUE_H
#define MAIN_FIXED_QUEUE_H

typedef struct {
  int size, item_length, current, first;
  char* buffer;
} FixedQueue;

void fixed_queue_init(FixedQueue *q, int size, int item_length, char *buffer);
void *fixed_queue_peek(FixedQueue* q);
void fixed_queue_pop(FixedQueue* q);
void fixed_queue_push(FixedQueue* q, const void *data);
void fixed_queue_reset(FixedQueue* q);
int fixed_queue_size(FixedQueue* q);
void *fixed_queue_peekn(FixedQueue* q, int offset);
int fixed_queue_get_index(FixedQueue* q, void *data, int (*cmp)(void*, void*));
void fixed_queue_move_to_top(FixedQueue* q, int offset, void *buffer);
char *fixed_queue_get_current_buffer_pointer(const FixedQueue* q);
void fixed_queue_move_next(FixedQueue* q);

#endif //MAIN_FIXED_QUEUE_H
