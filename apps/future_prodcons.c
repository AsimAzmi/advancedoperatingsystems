#include <xinu.h>
#include <future.h>


uint future_prod(future_t* fut, char* value) {
  //kprintf("\n future_prod : called");
  int* nptr = (int*) value;
  int status = (int) future_set(fut, value);
  if ( status < 1)
  {
    kprintf("\n future_set failed \n");
    return -1;
  }
  kprintf("\nProduced %d\n", *nptr);
  return OK;
}

uint future_cons(future_t* fut) {
  int i, status;
  //kprintf("\n inside future_cons");
  status = (int) future_get(fut, (char*) &i);
  if (status < 1) {
    printf("\n future_get failed\n");
    return -1;
  }
  kprintf("\nConsumed %d\n", i);

  return OK;
}