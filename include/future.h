#include<stddef.h>
#include<xinu.h>

#ifndef _FUTURE_H_
#define _FUTURE_H_
#define QUEUE_SIZE 20

typedef enum {
  FUTURE_EMPTY,
  FUTURE_WAITING,
  FUTURE_READY
} future_state_t;

typedef enum {
  FUTURE_EXCLUSIVE,
  FUTURE_SHARED,
  FUTURE_QUEUE
} future_mode_t;

typedef struct future_t {
  char *data;
  uint size;
  future_state_t state;
  future_mode_t mode;
  pid32 pid;
  pid32 set_queue[QUEUE_SIZE];
  pid32 get_queue[QUEUE_SIZE];
  int32 front_s, rear_s, front_g, rear_g;
} future_t;

 //Interface for the Futures system calls 
future_t* future_alloc(future_mode_t mode, uint size, uint nelems);
syscall future_free(future_t*);
syscall future_get(future_t*, char*);
syscall future_set(future_t*, char*);

//Interface for process queues.
void set_queue_insert(future_t*, pid32);
pid32 set_queue_remove( future_t*);
void  get_queue_insert(future_t*, pid32);
pid32 get_queue_remove(future_t*);


//prodcons definition
uint future_prod(future_t*, char*);
uint future_cons(future_t*);

#endif  
// _FUTURE_H_ 