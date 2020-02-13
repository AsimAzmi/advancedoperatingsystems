/* xsh_run.c - xsh_run */


#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <prodcons_bb.h>
#include <string.h>

/* xsh_run - take argument to map to a function and run */

// definition of array, semaphores and indices
int read_buffer;
int write_buffer;
sid32 mutex;
sid32 consumed;
sid32 produced;
int buffer[5];


void prodcons_bb(int nargs, char *args[]) {
  
   	
  //create and initialize semaphores to necessary values
    mutex = semcreate(1);
    consumed = semcreate(0);
    produced = semcreate(1);
   
  //initialize read and write indices for the queue
  read_buffer = -1;
  write_buffer = -1;
  printf("\n read :  %d \n", read_buffer);
  printf("\n write : %d \n", write_buffer); 
  //read = 0;
  // write = 0;

  //logs
  /*printf("\n prodcons_bb function called \n");
  printf("\n #produce %d", atoi(args[1]));
  printf("\n #consume %d", atoi(args[2]));
  printf("\n #loop_produce %d", atoi(args[3]));
  printf("\n #loop_consume %d", atoi(args[4]));*/ 
  int n_prod = atoi(args[1]);
  int n_cons = atoi(args[2]);
  int l_prod = atoi(args[3]);
  int l_cons = atoi(args[4]);
  
  //resume ( create(consumer_bb,1024,20, "cons", 3 , l_cons));
  //resume ( create(producer_bb, 1024, 20, "prod",3, l_prod));
  
  for ( int i = 0; i < n_cons; i++)
  {
	  char buff[20];
	  sprintf(buff,"%s%d","consumer",i);
	 // printf("thread name %s \n",buff);
	  resume (create(consumer_bb,1024,20, buff,1,l_cons));
  }
  for ( int i = 0; i < n_prod; i++)
  {
	  char buff[20];
	  sprintf(buff,"%s%d","producer",i);
	  resume (create(producer_bb, 1024,20,buff,1,l_prod));			  
  }
  
  //create producer and consumer processes and put them in ready queue
  //  resume( create(producer, 1024, 20, "producer", 1, count));
  //  resume( create(consumer, 1024, 20, "consumer", 1, count));
   printf("\n exiting prodcons_bb command \n");	

}

shellcmd xsh_run(int nargs, char *args[])
{
    if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0))
    {
      printf("prodcons_bb\n");
      //printf("my_function_2\n");
      return OK;
    }

    /* This will go past "run" and pass the function/process name and its
    * arguments.
    */
    args++;
    nargs--;

    if(strncmp(args[0], "prodcons_bb", 11) == 0)
    {
      /* create a process with the function as an entry point. */
      //resume (create((void *)my_function_1, 4096, 20, "my_func_1", 2, nargs, args));
      /*printf("\n prodcons function will be called \n");
      printf("\n #consume %d",atoi(args[1]));
      printf("\n #produce %d",atoi(args[2]));
      printf("\n #loop_consume %d", atoi(args[3]));
      printf("\n #loop_produce %d \n", atoi(args[4]));*/
      resume ( create((void *)prodcons_bb, 4096, 10, "prodcons_bb",2, nargs, args));
      printf("\n exiting main command \n");
      return OK;
    }
    else
    {
	    printf("\n The given function is not supported. Kindly check 'run list' command for list of functions \n");
	    return OK;
    }
}
