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
		kprintf("\n error returned by future_alloc: ");
		//printfS("%s " SYSERR);
	}
	else
	{
		kprintf("\n memory allocated by future_alloc");
		if( future_struct_addr->mode == FUTURE_EXCLUSIVE)
		{
			kprintf("\n Mode is future FUTURE_EXCLUSIVE. No queue reuired. : %d \n", future_struct_addr->mode);

		} 
		else if (future_struct_addr->mode == FUTURE_SHARED)
		{
			kprintf("\n Mode is future FUTURE_SHARED. Queue required %d \n", future_struct_addr->mode);
			//front_s, rear_s, front_g, rear_g;
			future_struct_addr->front_s = 0;
			future_struct_addr->rear_s = 0;
			future_struct_addr->front_g = 0;
			future_struct_addr->rear_g = 0;
		}
	}

	restore(mask);
	return future_struct_addr;

}



syscall future_get(future_t* future_t, char* data)
{
	intmask mask;
	mask = disable();
	kprintf("\ninside future_get \n");

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
			kprintf("future_get: Process suspended");
			suspend(future_t->pid);
			kprintf("future_get: Process resumed");
			
			*data = future_t->data;
			future_t->state = FUTURE_EMPTY;
			kprintf("future_get: Value get. State changed to EMPTY.");
		}
		else if ( future_t->state == FUTURE_READY)
		{
			*data = future_t->data;
			future_t->state = FUTURE_EMPTY;
			kprintf("\n future_get: Value get. State changed to EMPTY.");

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
			printf("\n future_get : FUTURE_SHARED : getpid() %s :", s);
			future_t->pid = getpid();
			future_t->state = FUTURE_WAITING;
			printf("\n future_get : FUTURE_SHARED : saved %s", (char *)proctab[future_t->pid].prname);
			get_queue_insert(future_t, getpid());
			suspend(future_t->pid);
			kprintf("future_get: FUTURE_SHARED : resumed %s ", (char *)proctab[future_t->pid].prname);
			*data = future_t->data;
		}

	}

	restore(mask);
	return OK;
}

syscall future_set(future_t* future_t, char* data)
{
	intmask mask;
	mask = disable();
	kprintf("\n inside future_set \n");
	if ( future_t->mode == FUTURE_EXCLUSIVE)
	{
		kprintf("\n future_set: FUTURE_EXCLUSIVE \n");
		if( future_t->state == FUTURE_READY)
		{
			kprintf("\nfuture_set: Error: Cannot set value. Future is in READY state");
			return SYSERR;
		}
		else
		{
			future_t->data = *data;
			future_t->state = FUTURE_READY;
			if ( future_t->pid != NULL)
			{
				resume(future_t->pid);
				kprintf("\n future_set : process resumed");
			}
			kprintf("\nfuture_set: Value set. State changed to READY.");
		}
	}
	else if (future_t->mode == FUTURE_SHARED)
	{
		kprintf("\n future_set: FUTURE_SHARED \n");
		if(future_t->state == FUTURE_READY)
		{
			kprintf("\nfuture_set: Error: Cannot set value. Future is in READY state");
			return SYSERR;
		}
		else if( future_t->state == FUTURE_WAITING)
		{
			future_t->data = *data;
			future_t->state = FUTURE_READY;
			while( future_t->front_g != future_t->rear_g)
			{
				pid32 pid = get_queue_remove(future_t);
				kprintf("future_set : future_shared : dequeue : %s ", (char*)proctab[pid].prname);
				resume(pid);
				
			}
		}
		else
		{
			future_t->data = *data;
			future_t->state = FUTURE_READY;
		}
	}
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
	kprintf("\n memory freed future_free");
	restore(mask);
	return OK;
		
}

// Queue Implementation
///front_s, rear_s, front_g, rear_g;
void set_queue_insert(future_t *future_struct, pid32 pid)
{
	if ( (future_struct->front_s == 0 && future_struct->rear_s == QUEUE_SIZE))
	{
		kprintf(" set_queue overflow");
		return SYSERR;	
	}
	else
	{
		future_struct->set_queue[future_struct->rear_s] = pid;
		
		if ((future_struct->rear_s == QUEUE_SIZE))
		{
			future_struct->rear_s = 0;
		}
		else
		{
			future_struct->rear_s += 1;
		}
		
	}
	 
}

pid32 set_queue_remove( future_t* future_struct)
{
	pid32 pid;
	if ( future_struct->front_s + 1 == future_struct->rear_s)	
	{
		kprintf(" set_queue underflow");
		return SYSERR;	
	}
	else
	{
		pid = future_struct->get_queue[future_struct->front_s];
		
		if ((future_struct->front_s == QUEUE_SIZE))
		{
			future_struct->front_s = 0;
		}
		else
		{
			future_struct->front_s += 1;
		}
		
	}
	return pid;
}
void  get_queue_insert(future_t* future_struct, pid32 pid)
{
	
	if (future_struct->rear_g == QUEUE_SIZE)
	{
		kprintf(" get_queue overflow");
		return SYSERR;	
	}
	else
	{
		future_struct->get_queue[future_struct->rear_g] = pid;
		kprintf("get_queue_insert: process inserted at %d : %s ", rear_g , (char *)proctab[future_struct->get_queue[rear_g]].prname);
		future_struct->rear_g = future_struct->rear_g + 1;				
	}
}

pid32 get_queue_remove(future_t* future_struct)
{
	pid32 pid = NULL;
	if ( future_struct->front_g == future_struct->rear_g)	
	{
		kprintf(" get_queue EMPTY");
		return SYSERR;	
	}
	else
	{
		pid = future_struct->get_queue[future_struct->front_g];
		kprintf("get_queue_remove: process removed at %d : %s ", front_g , (char *)proctab[pid].prname);
		future_struct->front_g = future_struct->front_g + 1;
	}

	return pid;
}
