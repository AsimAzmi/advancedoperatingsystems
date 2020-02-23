/* xsh_run.c - xsh_run */


#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
#include <prodcons_bb.h>
#include <string.h> 
#include <future.h>

/* xsh_run - take argument to map to a function and run */

// definition of array, semaphores and indices
int read_buffer;
int write_buffer;
sid32 mutex;
sid32 consumed;
sid32 produced;
int buffer[5];
int one;
int two;
int zero;
future_t **fibfut;


void prodcons_bb(int nargs, char *args[]) {
  
   	
  //create and initialize semaphores to necessary values
    mutex = semcreate(1);
    consumed = semcreate(0);
    produced = semcreate(5);
   
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

void future_test(int nargs, char *args[])
{
  
  one = 1;
  two = 2;
 
  if ( nargs == 2 && strncmp(args[1], "-pc", 3) == 0)
  {
    //kprintf("\n future_test : prodcons snippet called" );
    
    future_t *f_exclusive, *f_shared;
    f_exclusive = future_alloc(FUTURE_EXCLUSIVE, sizeof(int), 1);
    f_shared    = future_alloc(FUTURE_SHARED, sizeof(int), 1);

    //kprintf("\n future_test :  Mode f_exclusive: %d",f_exclusive->mode);
    //kprintf("\n future_test : Mode f_shared: %d ",f_shared->mode);

    // Test FUTURE_EXCLUSIVE
    resume( create(future_cons, 1024, 20, "fcons1", 1, f_exclusive) );
    resume( create(future_prod, 1024, 20, "fprod1", 2, f_exclusive, (char*) &one) );

    // Test FUTURE_SHARED
    resume( create(future_cons, 1024, 20, "fcons2", 1, f_shared) );
    resume( create(future_cons, 1024, 20, "fcons3", 1, f_shared) );
    resume( create(future_cons, 1024, 20, "fcons4", 1, f_shared) );
    resume( create(future_cons, 1024, 20, "fcons5", 1, f_shared) );
    resume( create(future_prod, 1024, 20, "fprod2", 2, f_shared, (char*) &two) );
    future_free(f_exclusive);
    future_free(f_shared);
  }
  else if ( nargs == 3 && strncmp(args[1], "-f", 2) == 0)
  {
     //kprintf("%s\n", args[1]);
     //kprintf("%s\n",args[2]);
     //kprintf("\n fibonachichi snippet will be called");
     int fib = -1, i;

    fib = atoi(args[2]);

    if (fib > -1) {
      int final_fib;
      int future_flags = FUTURE_SHARED; // TODO - add appropriate future mode here
       
      // create the array of future pointers
      if ((fibfut = (future_t **)getmem(sizeof(future_t *) * (fib + 1)))
          == (future_t **) SYSERR) {
        printf("getmem failed\n");
        return(SYSERR);
      }

      // get futures for the future array
      for (i=0; i <= fib; i++) {
        if((fibfut[i] = future_alloc(future_flags, sizeof(int), 1)) == (future_t *) SYSERR) {
          printf("future_alloc failed\n");
          return(SYSERR);
        }
      }

      // spawn fib threads and get final value
      // TODO - you need to add your code here
      zero = 0;
      one = 1;

      for ( i=0; i <= fib; i++ ) {
        char buff[20];
        //kprintf(buff,"%s%d","fibelement",i);
        resume( create(ffib, 1024, 20, "fib", 1, i) );
      }

      future_get(fibfut[fib], (char*) &final_fib);

      for (i=0; i <= fib; i++) {
        future_free(fibfut[i]);
      }

      freemem((char *)fibfut, sizeof(future_t *) * (fib + 1));
      printf("\nNth Fibonacci value for N=%d is %d\n", fib, final_fib);
      return(OK);
    }
  }
  else
  {
    kprintf("\n check paramters");
  }
 

  printf("\n future_test meethod thread ends.");
}

shellcmd xsh_run(int nargs, char *args[])
{
    if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0))
    {
      printf("prodcons_bb\n");
      printf("future_test");
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
      resume ( create((void *)prodcons_bb, 4096, 10, "prodcons_bb",2, nargs, args));
      printf("\n exiting main command \n");
      return OK;
    }
    else if(strncmp(args[0], "future_test", 11) == 0)
    {
      //printf("\nfuture_test called\n");
      resume ( create((void *)future_test, 4096, 10, "future_test",2, nargs, args));
    }
    else
    {
	    printf("\n The given function is not supported. Kindly check 'run list' command for list of functions \n");
	    return OK;
    }
}
