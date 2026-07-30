#include <stdio.h>
#include <fixed_queue.h>
#include <string.h>

#define QUEUE_SIZE 10
#define ITEM_SIZE 16

static char buffer[QUEUE_SIZE * ITEM_SIZE];
static char temp_buffer[ITEM_SIZE];

static Queue q;

int main(void)
{
  queue_init(&q, QUEUE_SIZE, ITEM_SIZE, buffer);
  queue_push(&q, "Test1");
  queue_push(&q, "Test2");
  queue_push(&q, "Test3");
  int idx = queue_get_index(&q, "Test4", (int(*)(void*,void*))strcmp);
  printf("idx(not exists) = %d\n", idx);
  idx = queue_get_index(&q, "Test2", (int(*)(void*,void*))strcmp);
  printf("idx(Test2) = %d\n", idx);
  idx = queue_get_index(&q, "Test3", (int(*)(void*,void*))strcmp);
  printf("idx(Test3) = %d\n", idx);
  idx = queue_get_index(&q, "Test1", (int(*)(void*,void*))strcmp);
  printf("idx(Test1) = %d\n", idx);
  queue_move_to_top(&q, 2, temp_buffer);
  idx = queue_get_index(&q, "Test2", (int(*)(void*,void*))strcmp);
  printf("idx(Test2) = %d\n", idx);
  idx = queue_get_index(&q, "Test3", (int(*)(void*,void*))strcmp);
  printf("idx(Test3) = %d\n", idx);
  idx = queue_get_index(&q, "Test1", (int(*)(void*,void*))strcmp);
  printf("idx(Test1) = %d\n", idx);
  return 0;
}