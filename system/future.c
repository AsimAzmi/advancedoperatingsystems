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
			
			

			/*while(1)
			{
				if(future_t->state == FUTURE_READY)
				{
					*data = future_t->data;
					future_t->state = FUTURE_EMPTY;
					kprintf("future_get: Value get. State changed to EMPTY.");
					break;
				}
		
			}*/
		}
		else if ( future_t->state == FUTURE_READY)
		{
			*data = future_t->data;
			future_t->state = FUTURE_EMPTY;
			kprintf("\n future_get: Value get. State changed to EMPTY.");

		}
	}

	restore(mask);
	return 0;
}

syscall future_set(future_t* future_t, char* data)
{
	intmask mask;
	mask = disable();
	kprintf("\n future_set \n");
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
	restore(mask);
	return 0;
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
	return 0;
		
}