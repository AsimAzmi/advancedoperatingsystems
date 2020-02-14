#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>
//#include <pthread.h>


void producer(int count) {

	int32 i;

	for ( i = 1; i <= count; i++)
	{
		n = i;
		printf("\n Produced : %d \n", n);
	}

}


void producer_bb(int count)
{
  int i;	
  for ( i = 0; i < count; i++)
  {
     wait(produced);	  
     wait(mutex);
     if ( (read_buffer == 0 && write_buffer == 4) || ( read_buffer == write_buffer + 1))
     {
	     kprintf("Produce : Queue OverFlow \n");
	     signal(produced);
	     return;
     }
     if ( read_buffer == -1 )
     {
	     read_buffer = 0;
	     write_buffer = 0;
     }
     if (write_buffer == 4)
     {
      		write_buffer = 0;
     }

     buffer[write_buffer] = i;
     char *s = proctab[getpid()].prname;

     kprintf("\n name : %s, write : %d \n", s, buffer[write_buffer++]);
     //printf("read_buffer value in producer %d \n", read_buffer);
     signal(mutex);
     signal(consumed);
  }	  

  // Iterate from 0 to count and for each iteration add iteration value to the global array `arr_q`, 
  // print producer process name and written value as,
  // name : producer_1, write : 8
}

