#ifndef _TSCDF_H_
#define _TSCDF_H_


#include <xinu.h>
#include <future.h>



typedef struct data_element {
  int32 time;
  int32 value;
} de;

typedef struct stream {
  sid32 spaces; //producer semaphore
  sid32 items; //consumeer sem
  sid32 mutex;
  int32 head;
  int32 tail;
  struct data_element *queue;
} stream;


int stream_proc(int nargs, char* args[]);
void stream_consumer(int32 id, struct stream *str);

int stream_proc_futures(int nargs, char* args[]);
void future_stream_consumer(int32 id, struct future_t *f);

void stream_producer();

 extern int num_streams;
 extern int work_queue_depth;
 extern int time_window;
 extern int output_time;

//extern stream **streams;
#endif