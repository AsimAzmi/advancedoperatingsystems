#include<future.h>
#include<xinu.h>
#include<stdio.h>
#include<stdlib.h>

//#include<unistd.h>



// nelems i.e number of elements will always be 1 for now. Its not used
future_t* future_alloc(future_mode_t mode, uint size, uint nelems)
{
	intmask mask;
	mask = disable();
	future_t *future_struct_addr = (future_t *) getmem(sizeof(future_t));
	future_struct_addr->mode = mode;
	future_struct_addr->state = FUTURE_EMPTY;

	if (SYSERR == future_struct_addr)
	{
		return(SYSERR);
		//kprintf("\n error returned by future_alloc: ");
		//printfS("%s " SYSERR);
	}
	else
	{
		//kprintf("\n future_alloc:  memory allocated by future_alloc");
		if( future_struct_addr->mode == FUTURE_EXCLUSIVE)
		{
			//kprintf("\n future_alloc: Mode is future FUTURE_EXCLUSIVE. No queue reuired. : %d \n", future_struct_addr->mode);

		} 
		else if (future_struct_addr->mode == FUTURE_SHARED)
		{
			//kprintf("\n future_alloc: Mode is future FUTURE_SHARED. Queue required %d \n", future_struct_addr->mode);
			//front_s, rear_s, front_g, rear_g;
			future_struct_addr->front_s = 0;
			future_struct_addr->rear_s = 0;
			future_struct_addr->front_g = 0;
			future_struct_addr->rear_g = 0;
		}
		else if (future_struct_addr->mode == FUTURE_QUEUE)
		{
			future_struct_addr->max_elems = nelems;
			future_struct_addr->count = 0;
			future_struct_addr->head =  -1;
			future_struct_addr->tail =  -1;
			future_struct_addr->size = size;
			future_struct_addr->front_s = -1;
			future_struct_addr->rear_s = -1;
			future_struct_addr->front_g = -1;
			future_struct_addr->rear_g = -1;

			if(future_struct_addr->data =(int *) getmem(size * nelems) == (int *) SYSERR)
   			{
   		  		printf("data queue allocation failed\n");
          		return(SYSERR);
   			}
   			//kprintf("\n tail %d ", future_struct_addr->tail );
   			//kprintf("\n  head %d", future_struct_addr->head );
   			//kprintf("\n total %d", size*nelems);

   			//future_struct_addr->data[0] = 10;
   			//future_struct_addr->data[1] = 20;
   			//future_struct_addr->data[2] = 30;
   			//char* headelemptr = future_struct_addr->data + (future_struct_addr->head * future_struct_addr->size);
			//char* tailelemptr = future_struct_addr->data + (future_struct_addr->tail * future_struct_addr->size);
   			//kprintf("\n value 1 %d", (future_struct_addr->data[0]));
   			//kprintf("\n value 2 %d", (future_struct_addr->data[1]));
   			//kprintf("\n value 3 %d", (future_struct_addr->data[2]));
   			

   			
		}
		else
		{
			kprintf("Incorrect mode");
			return SYSERR;
		}
	}

	kprintf("\n");
	restore(mask);
	return future_struct_addr;

}



syscall future_get(future_t* future_t, char* data)
{
	intmask mask;
	mask = disable();
	//kprintf("\ninside future_get \n");

	if ( future_t->mode == FUTURE_EXCLUSIVE)
	{
		if( future_t->state == FUTURE_WAITING)
		{
			kprintf("Error: Cannot get value. Future is in Waiting state");
			return SYSERR;
		}
		else if ( future_t->state == FUTURE_EMPTY)
		{
			future_t->state = FUTURE_WAITING;
			future_t->pid = getpid();
			//kprintf("future_get: Process suspended");
			suspend(future_t->pid);
			//kprintf("future_get: Process resumed");
			
			*data = future_t->data;
			future_t->state = FUTURE_EMPTY;
			//kprintf("future_get: Value get. State changed to EMPTY.");
		}
		else if ( future_t->state == FUTURE_READY)
		{
			*data = future_t->data;
			future_t->state = FUTURE_EMPTY;
			//kprintf("\n future_get: Value get. State changed to EMPTY.");

		}
	}
	else if ( future_t->mode == FUTURE_SHARED)
	{
		if( future_t->state == FUTURE_READY)
		{
			*data = future_t->data;
		}
		else
		{	
			char *s = proctab[getpid()].prname;
			//kprintf("\n future_get : FUTURE_SHARED : getpid() %s :", s);
			future_t->pid = getpid();
			future_t->state = FUTURE_WAITING;
			//printf("\n future_get : FUTURE_SHARED : saved %s", (char *)proctab[future_t->pid].prname);
			get_queue_insert(future_t, getpid());
			suspend(future_t->pid);
			//kprintf("future_get: FUTURE_SHARED : resumed %s ", (char *)proctab[future_t->pid].prname);
			*data = future_t->data;
		}
		//kprintf("data :  %d \n",*data);

	}
	else if ( future_t->mode = FUTURE_QUEUE)
	
	{
		//data_queue_remove(future_t, data);
		//kprintf("\n get_data %d", *data);
		if ( future_t->state == FUTURE_READY)
		//if ( !(isGetQueueEmpty(future_t)))
		{
			data_queue_remove(future_t, data);
			if (isDataQueueEmpty(future_t))
			{
				future_t->state = FUTURE_EMPTY;
			}
			if( !(isSetQueueEmpty(future_t)))
			{
				pid32 pid = set_queue_remove(future_t);
				resume(pid);
			}
		}
		else
		{
			char *s = proctab[getpid()].prname;
			//kprintf("\n future_get : FUTURE_SHARED : getpid() %s :", s);
			future_t->pid = getpid();
			future_t->state = FUTURE_WAITING;
			//printf("\n future_get : FUTURE_SHARED : saved %s", (char *)proctab[future_t->pid].prname);
			get_queue_insert(future_t, getpid());
			//kprintf("\n consumer suspended : %d \n ", future_t->pid);
			suspend(future_t->pid);
			//kprintf("future_get: FUTURE_SHARED : resumed %s ", (char *)proctab[future_t->pid].prname);
			//kprintf("\nconsumer resumed: %d\n", future_t->pid);
			data_queue_remove(future_t, data);

			if (isDataQueueEmpty(future_t))
			{
				future_t->state = FUTURE_EMPTY;
			}
			if( !(isSetQueueEmpty(future_t)))
			{
				pid32 pid = set_queue_remove(future_t);
				resume(pid);
			}
		}
	}

	restore(mask);
	return OK;
}

syscall future_set(future_t* future_t, char* data)
{
	intmask mask;
	mask = disable();
	//kprintf("data : %d", *data);
	//kprintf("mode %s", future_t->state);
	//kprintf("\n inside future_set \n");
	if ( future_t->mode == FUTURE_EXCLUSIVE)
	{
		//kprintf("\n future_set: FUTURE_EXCLUSIVE \n");
		if( future_t->state == FUTURE_READY)
		{
			//kprintf("\nfuture_set: Error: Cannot set value. Future is in READY state");
			return SYSERR;
		}
		else
		{
			future_t->data = *data;
			future_t->state = FUTURE_READY;
			if ( future_t->pid != NULL)
			{
				resume(future_t->pid);
				//kprintf("\n future_set : process resumed");
			}
			//kprintf("\nfuture_set: Value set. State changed to READY.");
		}
	}
	else if (future_t->mode == FUTURE_SHARED)
	{
		//kprintf("\n future_set: FUTURE_SHARED \n");
		if(future_t->state == FUTURE_READY)
		{
			//kprintf("\nfuture_set: Error: Cannot set value. Future is in READY state");
			return SYSERR;
		}
		else if( future_t->state == FUTURE_WAITING)
		{
			kprintf("state watiing");
			future_t->data = *data;
			future_t->state = FUTURE_READY;
			while( future_t->front_g != future_t->rear_g)
			{
				pid32 pid = get_queue_remove(future_t);
				//kprintf("future_set : future_shared : dequeue : %s ", (char*)proctab[pid].prname);
				resume(pid);
				
			}
		}
		else
		{
			kprintf("state ready");
			future_t->data = *data;
			kprintf("%d",*data);
			future_t->state = FUTURE_READY;
		}
	}
	else if ( future_t->mode == FUTURE_QUEUE)
	{
		//Not sure when this condition arrives
		if(future_t->state == FUTURE_READY)
		{
			//return SYSERR;
		}
		
		if ( future_t->state == FUTURE_WAITING)
		{
			//kprintf("\n inset->time %d\n", data[0]);
			data_queue_insert(future_t, data);
			future_t->state = FUTURE_READY;
			if ( ! (isGetQueueEmpty(future_t)))
			{
				pid32 pid = get_queue_remove(future_t);
				
				resume(pid);				
			}

		} 
		else
		{
			if (isDataQueueFull(future_t))
			{
				//kprintf("work qeue is full");
				future_t->pid = getpid();
				set_queue_insert(future_t,getpid());
				suspend(future_t->pid);
				data_queue_insert(future_t, data);
				future_t->state = FUTURE_READY;
				if ( ! (isGetQueueEmpty(future_t)) )
				{
					pid32 pid = get_queue_remove(future_t);
					//kprintf("\nconsumer resumed: %d\n", pid);
					resume(pid);				
				}
			}
			else
			{
				data_queue_insert(future_t, data);
				future_t->state = FUTURE_READY;
				if ( ! (isGetQueueEmpty(future_t)))
				{
					pid32 pid = get_queue_remove(future_t);
					//kprintf("\nconsumer resumed: %d\n", pid);
					resume(pid);				
				}
			}
		}
	}
	//data_queue_insert(future_t,data);

	restore(mask);
	return OK;
}

syscall future_free(future_t* f)
{
	intmask mask;
	mask = disable();
	
	if(freemem((char *)f,sizeof(future_t)) == SYSERR) 
	{
		f = NULL;
		return SYSERR;
	}	
		
	f = NULL;
	//kprintf("\n memory freed future_free");
	restore(mask);
	return OK;
		
}


bool isDataQueueFull(future_t *f)
{
	if ( (f->head == (f->tail +1)) || (f->head == 0 && f->tail == f->max_elems - 1))
		return true;
	else
		return false;
}


bool isDataQueueEmpty(future_t *f)
{
	if ( f->head == -1)
		return true;
	else
		return false;
}

bool isSetQueueFull(future_t *f)
{
	if ( (f->front_s == (f->rear_s +1)) || (f->front_s == 0 && f->rear_s == QUEUE_SIZE - 1))
		return true;
	else
		return false;
}


bool isSetQueueEmpty(future_t *f)
{
	if ( f->front_s == -1)
		return true;
	else
		return false;
}
bool isGetQueueFull(future_t *f)
{
	if ( (f->front_g == (f->rear_g +1)) || (f->front_g == 0 && f->rear_g == QUEUE_SIZE - 1))
		return true;
	else
		return false;
}


bool isGetQueueEmpty(future_t *f)
{
	if ( f->front_g == -1)
		return true;
	else
		return false;
}

void data_queue_insert(future_t *f, char* data)
{
	
	//char* tailelemptr = f->data + (f->tail * f->size);

	//kprintf("\n Inside insert");
	if (isDataQueueFull(f))
	{
		kprintf("\n Data Queue full");
		return SYSERR;
	}
	else
	{
		if (f->head == -1)
		{
				f->head = 0;
		}
		
		//kprintf("\n tail before insert %d", f->tail);
		f->tail = (f->tail + 1) % f->max_elems;
		char* tailelemptr = f->data + (f->tail * f->size);
		memcpy(tailelemptr ,data, f->size);
		//kprintf("\n inserted time : %d, value : %d, process %d \n",*(tailelemptr), *(tailelemptr+sizeof(int)), getpid());
		//kprintf("i\nndataqueue time%d\n", tailelemptr[0]);
		
		//f->data[f->tail] = *data;
		

		
		f->count++;
	}
	
}

void data_queue_remove(future_t *f, char *data)
{
	//kprintf("\n Inside remove");
	if( isDataQueueEmpty(f))
	{
		kprintf("\n DataQueue Empty" );
		return SYSERR;
	}
	else
	{
		//int head_data = f->data[f->head];
		char* headelemptr = f->data + (f->head * f->size);
		memcpy(data, headelemptr, f->size);
		//kprintf("\n head %d", f->head);
		//kprintf("\n data %d", f->data[f->head]);
		if (f->head == f->tail)
		{
			f->head = -1;
			f->tail = -1;
		}
		else
		{
			f->head = (f->head + 1) % f->max_elems;
			f->count--;	
		}	
		//memcpy(data, &head_data, sizeof(int));	
	}
	
}

// Queue Implementation
///front_s, rear_s, front_g, rear_g;
void set_queue_insert(future_t *f, pid32 pid)
{
	if (isSetQueueFull(f))
	{
		kprintf("\n Set Queue full");
		return SYSERR;
	}
	else
	{
		if (f->front_s == -1)
		{
				f->front_s = 0;
		}
		f->rear_s = (f->rear_s + 1) % QUEUE_SIZE;
		f->set_queue[f->rear_s] = pid;
		
	}
	 
}

pid32 set_queue_remove( future_t* f)
{
	pid32 pid;
	//kprintf("\n Inside remove");
	if( isSetQueueEmpty(f))
	{
		kprintf("\n SetQueue Empty" );
		return SYSERR;
	}
	else
	{
		pid = f->set_queue[f->front_s];
		if (f->front_s == f->rear_s)
		{
			f->front_s = -1;
			f->rear_s = -1;
		}
		else
		{
			f->front_s = (f->front_s + 1) % QUEUE_SIZE;	
		}	
		
		return pid;
	}
}


void  get_queue_insert(future_t* f, pid32 pid)
{
	if (isGetQueueFull(f))
	{
		kprintf("\n Get Queue full");
		return SYSERR;
	}
	else
	{
		if (f->front_g == -1)
		{
				f->front_g = 0;
		}
		f->rear_g = (f->rear_g + 1) % QUEUE_SIZE;
		f->get_queue[f->rear_g] = pid;
		
	}
	
}

pid32 get_queue_remove(future_t* f)
{
	pid32 pid;
	//kprintf("\n Inside remove");
	if( isGetQueueEmpty(f))
	{
		kprintf("\n GetQueue Empty" );
		return SYSERR;
	}
	else
	{
		pid = f->get_queue[f->front_g];
		if (f->front_g == f->rear_g)
		{
			f->front_g = -1;
			f->rear_g = -1;
		}
		else
		{
			f->front_g = (f->front_g + 1) % QUEUE_SIZE;	
		}	
		
		return pid;
	}

}
