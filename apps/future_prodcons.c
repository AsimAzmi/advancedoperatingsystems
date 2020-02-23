#include <xinu.h>
#include <future.h>


uint future_prod(future_t* fut, char* value) {
  //kprintf("\n future_prod : called");
  int* nptr = (int*) value;
  future_set(fut, value);
  kprintf("future_prod: Produced %d\n", *nptr);
  return OK;
}

uint future_cons(future_t* fut) {
  int i, status;
  //kprintf("\n inside future_cons");
  status = (int) future_get(fut, (char*) &i);
  if (status < 1) {
    printf("future_get failed\n");
    return -1;
  }
  kprintf("future_cons: Consumed %d\n", i);

  return OK;
}