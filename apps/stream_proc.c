//#include <tscdf.h>
//#include <stdlib.h>
//#include <stdio.h>
#include <tscdf_a5.h>
#include "tscdf.h"
//#include "tscdf_input.h"



int work_queue_depth;
int time_window;
int num_streams;
int output_time;
uint pcport;



void stream_consumer(int32 id, struct stream *str)
{			
	//kprintf("%d Consumer Create \n", id);
	struct tscdf *tscdf_pointer  = tscdf_init(time_window);
	int count = 0;
	
	kprintf("stream_consumer id:%d (pid:%d) \n",id,getpid());

	while(1)
	{
		count++;
		wait(str->items);
		wait(str->mutex);
		
		str->head = (str->head+1) % work_queue_depth;
		//kprintf("stream_consumer id:%d (pid:%d)  time:%d value:%d \n",id,getpid(), str->queue[str->head].time, str->queue[str->head].value);
		
		

	

		signal(str->mutex);
		signal(str->spaces);

		// quartile code
		tscdf_update(tscdf_pointer, str->queue[str->head].time, str->queue[str->head].value);

		

		if(count == output_time)	
		{
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
		// quartile code ends		

		if (str->queue[str->head].time == 0 && str->queue[str->head].value == 0)
		{
			//kprintf("%d consumer exits \n", id);
			
			kprintf("stream_consumer exiting \n");
			ptsend(pcport, getpid());
			break;
		}


	}	

	return;
				
}



int stream_proc(int nargs, char* args[]) 
{

  // ports and time

  

  ulong secs, msecs, time;
  secs = clktime;
  msecs = clkticks;




  int i = sizeof(args) / sizeof(char*);
  //printf("%d\n ", i);
  char c;
  char *ch;
  ///int i = 
  //ulong secs, msecs, time;
  //secs = clktime;
  //msecs = clkticks;



  char usage[] = "Usage: -s num_streams -w work_queue_depth -t time_window -o output_time\n";

  /* Parse arguments out of flags */
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


   //ports
    if((pcport = ptcreate(num_streams)) == SYSERR) {
      printf("ptcreate failed\n");
      return(-1);
 	 }
  	//printf("\n %d port \n", pcport);
	 // ports	


   //create an array of streams.
   //kprintf("creating an array of streams\n");
   struct stream streams[num_streams];	

  

  // Create streams
   for (i = 0; i < num_streams; i++) 
   {
   		
   		if((streams[i].queue = (de *) getmem(sizeof(de) * (work_queue_depth ))) == (de *) SYSERR)
   		{
   		  printf("streams queue allocation failed\n");
          return(SYSERR);
   		}
   		streams[i].mutex = semcreate(1);
   		streams[i].spaces = semcreate(work_queue_depth);
   		streams[i].items = semcreate(0);
   		streams[i].head = -1;
   		streams[i].tail = -1;

   		/*struct data_element data[work_queue_depth];
   		streams[i].queue = data;*/
   		//printf("\n size of %d stream : %d", i , sizeof(streams[i].queue));
   		//printf("\n adrress of %d stream : %d", i , streams[i].queue);
   		
   }



	// dummy consume value  	 
    // Create consumer processes and initialize streams
    // Use `i` as the stream id.
    for ( i = 0; i < num_streams; i++)
  	{
  		char buff[2];
  		kprintf(buff,"%d",i);
  		resume ( create((void *)stream_consumer, 4096, 10, buff ,2, (int32)i, &streams[i]));
  	}
 
   

    //kprintf("size %d \n", sizeof(stream_input)/sizeof(stream_input[0]));
     // Parse input header file data and populate work queue
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
    	

    	
    	wait(streams[st].spaces);
    	wait(streams[st].mutex);
    	streams[st].tail = ((streams[st].tail + 1 ) % work_queue_depth);
    	streams[st].queue[streams[st].tail].time = ts;
    	streams[st].queue[streams[st].tail].value = v;
    	//kprintf("Producer id: %d time: %d value: %d \n", st, streams[st].queue[streams[st].tail].time, streams[st].queue[streams[st].tail].value );	
    	//kprintf(" Producer \n ");	
	    signal(streams[st].mutex);
    	signal(streams[st].items);
		
    }

    //check values
    /*for ( i = 0; i < num_streams; i++) 
  	{
  		for ( int j = 0; j < work_queue_depth; j++)
  		{
  			kprintf("ID: %d , Time: %d , Value: %d,   \n ", i, streams[i].queue[j].time, streams[i].queue[j].value);
  		}
	}*/




    // ports and time
    for(i=0; i < num_streams; i++) {
      uint32 pm;
      //kprintf("\n port listenin started ...............................");
      pm = ptrecv(pcport);
      kprintf("process %d exited\n", pm);
  	}

  	ptdelete(pcport, 0);

  	time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
  	printf("time in ms: %u\n", time);
  	//ports and time

  return 0;
}




/*streams[0].queue[0].time = 0;
   	 streams[0].queue[1].time = 1;
   	 streams[0].queue[2].time = 2;

   	 printf("\n  Single stream queue");
   	 printf("\n %d stream0  \n", streams[0].queue[0].time );
   	 printf("\n %d stream1  \n", streams[0].queue[1].time );
   	 printf("\n %d stream1  \n", streams[0].queue[2].time );

   	 streams[0].head = 0;
   	 streams[1].head = 1;
   	 streams[2].head = 2;


   	 printf("\n  multipel stream head");
   	  printf("\n %d stream0  \n", streams[0].head);
   	  printf("\n %d stream1  \n", streams[1].head);
   	  printf("\n %d stream2  \n", streams[2].head);

   	 streams[0].queue[0].time = 0;
   	 streams[1].queue[0].time = 1;
   	 streams[2].queue[0].time = 2;

   	 printf("\n  multile stream queue");
   	 printf("\n %d stream0  \n", streams[0].queue[0].time );
   	 printf("\n %d stream1  \n", streams[1].queue[0].time );
   	 printf("\n %d stream2  \n", streams[2].queue[0].time );*/