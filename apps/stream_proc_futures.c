//#include <tscdf.h>
//#include <stdlib.h>
//#include <stdio.h>
#include <tscdf_a5.h>
#include "tscdf.h"
//#include<future.h>
//#include "tscdf_input.h"



int work_queue_depth;
int time_window;
int num_streams;
int output_time;
uint pcport;



void future_stream_consumer(int32 id, struct future_t *f)
{     
  struct tscdf *tscdf_pointer = tscdf_init(time_window);
  int count = 0;
  kprintf("stream_consumer_future id:%d (pid:%d) \n",id,getpid());
  while(1)
  {
    count++;
    struct data_element *ptr = getmem(sizeof(de));
    int status = (int) future_get(f, ptr);
    //kprintf("\ntime %d, value %d\n", ptr->time, ptr->value);
    tscdf_update(tscdf_pointer, ptr->time, ptr->value);
    //kprintf("\ntscdf update by consumer %d :\n", getpid());

    if(count == output_time)  
    {
      //kprintf("\nprinting first batch\n");
      count = 0;
      char output[30];
      int32 *qarray;
      qarray = tscdf_quartiles(tscdf_pointer);

        if(qarray == NULL) {
              kprintf("tscdf_quartiles returned NULL\n");
              continue;
            }
    
        sprintf(output, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
        kprintf("%s\n", output);
        freemem((char *)qarray, (6*sizeof(int32)));
    }

    if (ptr->time == 0 && ptr->value == 0)
    {
      //kprintf("%d consumer exits \n", id);
      
      //kprintf("stream_consumer exiting \n");
      ptsend(pcport, getpid());
      if((freemem((char *)f->data,sizeof(sizeof(de) * (work_queue_depth ))) == SYSERR) || (tscdf_free(tscdf_pointer)== SYSERR)) 
      {
          f->data = NULL;
          tscdf_pointer = NULL;
          return SYSERR;
      }
      f->data = NULL;
      tscdf_pointer = NULL; 


      break;
    }

  }       
}

int stream_proc_futures(int nargs, char* args[])
 {
  ulong secs, msecs, time;
  secs = clktime;
  msecs = clkticks;
  int i = sizeof(args) / sizeof(char*);
  char c;
  char *ch;
  char usage[] = "Usage: -s num_streams -w work_queue_depth -t time_window -o output_time\n";

  // Parse arguments
  /* if not even # args, print error and exit */
  if (!(nargs % 2)) {
    printf("%s", usage);
    return(-1);
  }
  else {
    i = nargs - 1;
    while (i > 0) {
      ch = args[i-1];
      c = *(++ch);
      
      switch(c) {
      case 's':
        num_streams = atoi(args[i]);
        //printf("%d\n ", num_streams );
        break;

      case 'w':
        work_queue_depth = atoi(args[i]);
        //printf("%d\n ", work_queue_depth );
        break;

      case 't':
        time_window = atoi(args[i]);
        //printf("%d\n ", time_window );
        break;
        
      case 'o':
        output_time = atoi(args[i]);
        //printf("%d\n ", output_time );
        break;

      default:
        printf("%s\n ", usage);
        return(-1);
      }

      i -= 2;
    }
   }
  // Parse arguments



  // Create array to hold `n_streams` number of futures
  //future_t **farray;
  struct future_t streams[num_streams];


  //ports
  if((pcport = ptcreate(num_streams)) == SYSERR) {
      printf("ptcreate failed\n");
      return(-1);
   }


  //Allocate futures
  // Future mode = FUTURE_QUEUE
  // Size of element = sizeof(struct data_element)
  // Number of elements = work_queue_depth
  for (i = 0; i < num_streams; i++) 
  {
    if((streams[i].data = (int *) getmem(sizeof(de) * (work_queue_depth ))) == (int *) SYSERR)
      {
        printf("streams queue allocation failed\n");
          return(SYSERR);
      }
      //kprintf("\n queue size %d\n", sizeof(de) * (work_queue_depth ));
      streams[i].max_elems = work_queue_depth;
      streams[i].count = 0;
      streams[i].head =  -1;
      streams[i].tail =  -1;
      streams[i].size = sizeof(de);
      //kprintf("size of de %d ", streams[i].size);
      streams[i].front_s = -1;
      streams[i].rear_s = -1;
      streams[i].front_g = -1;
      streams[i].rear_g = -1; 
      streams[i].mode = FUTURE_QUEUE;
      streams[i].state = FUTURE_EMPTY;

   }

   //  Create consumer processes  
   // Use `i` as the stream id.
   for ( i = 0; i < num_streams; i++)
    {
      char buff[2];
      kprintf(buff,"%d",i);
      resume ( create((void *)future_stream_consumer, 4096, 10, buff ,2, (int32)i, &streams[i]));
    }
  

  // Parse input header file data and set future values
    for ( i = 0; i < n_input; i++)
    {
      int ts, v;
      char *a = (char *)stream_input[i];
      int st = atoi(a);
      //printf("st  %d ", st);
      while (*a++ != '\t')  
      {
         ts = atoi(a);
          
      }
      //printf("ts  %d ", ts);
      while (*a++ != '\t')
      {
         v = atoi(a);
        
      } 
      //kprintf("v  %d ", v);     
      //kprintf("******************************** \n ");
      
      struct data_element *ptr = getmem(sizeof(de));
      ptr->time = ts;
      ptr->value = v;
      future_set((char*)&streams[st], ptr);
      //data_queue_insert((char*)&streams[st], ptr);

    }




  // Wait for all consumers to exit
      for(i=0; i < num_streams; i++) {
      uint32 pm;
      //kprintf("\n port listenin started ...............................");
      pm = ptrecv(pcport);
      kprintf("process %d exited\n", pm); 
    }

    ptdelete(pcport, 0);
    time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
    kprintf("time in ms: %u\n", time);
    //free futures array
  return 0;
}