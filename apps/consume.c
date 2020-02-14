#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>
void consumer(int count) {

	for ( int i = 0; i <= count; i++)
	{
		printf("\n Consumed :  %d \n",n);
	}

}

void consumer_bb (int count)
{
	for ( int i = 0; i < count; i++)
	{
	  wait(consumed);	
	 // printf("read_buffer value in consumer %d \n", read_buffer);
	  if ( read_buffer == -1 )
	  {
		  kprintf("\n Consuming: Queue empty \n"); 
		  signal(consumed);
		  return;
	  }
	  char *s = proctab[getpid()].prname;

	  kprintf("\n name : %s, read : %d \n \n", s , buffer[read_buffer++]);
	  if ( read_buffer == write_buffer)
	  {
		  write_buffer = -1;
		  read_buffer = -1;
	  }
	   if ( read_buffer == 4 ) // Buffer length is hardcode as mentioned;
	   {
	       read_buffer = 0;
	   }  
	  
	   signal(produced);
	  // signal(mutex);
	}
}


