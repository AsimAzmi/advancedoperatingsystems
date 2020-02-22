#include <xinu.h>
#include <future.h>


uint future_prod(future_t* fut, char* value) {
  kprintf("\ninsiede future_prod\n");
  int* nptr = (int*) value;
  future_set(fut, value);
   kprintf("future_prod: Produced %d\n", *nptr);
  return OK;
}

uint future_cons(future_t* fut) {
  int i, status;
  kprintf("inside future_cons");
  status = (int) future_get(fut, (char*) &i);
  if (status < 1) {
    printf("future_get failed\n");
    return -1;
  }
  kprintf("Consumed %d\n", fut->data);

  return OK;
}