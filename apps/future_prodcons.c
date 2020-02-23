#include <xinu.h>
#include <future.h>


uint future_prod(future_t* fut, char* value) {
  kprintf("\ninsiede future_prod");
  int* nptr = (int*) value;
  kprintf("\n future_prod: mode  %s",fut->mode);
  future_set(fut, value);
  kprintf("\nfuture_prod: Produced %d\n", *nptr);
  return OK;
}

uint future_cons(future_t* fut) {
  int i, status;
  kprintf("\n inside future_cons");
  kprintf("\n future_cons: mode %d \n",fut->mode);
  status = (int) future_get(fut, (char*) &i);
  if (status < 1) {
    printf("future_get failed\n");
    return -1;
  }
  kprintf("Consumed %d\n", i);

  return OK;
}