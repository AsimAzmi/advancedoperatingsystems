#include<future.h>
#include<xinu.h>
#include<stdio.h>
#include<stdlib.h>



// nelems i.e number of elements will always be 1 for now. Its not used
future_t* future_alloc(future_mode_t mode, uint size, uint nelems)
{
	intmask mask;
	mask = disable();
	future_t *future_struct_addr = (future_t*) getmem(sizeof(future_t));
	if (SYSERR == future_struct_addr)
	{
		printf("\n error returned by future_alloc: ");
		//printfS("%s " SYSERR);
		}
	else
	{
		printf("\n memory allocated by future_alloc");
		if( mode == FUTURE_EXCLUSIVE)
		{
			printf("\n Mode is future FUTURE_EXCLUSIVE. No queue reuired\n");
		} 
		else if (mode == FUTURE_SHARED)
		{
			printf("\n Mode is future FUTURE_SHARED. Queue required queue reuired\n");
		}
	}

	restore(mask);
	return future_struct_addr;

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
	printf("\n memory freed future_free");
	restore(mask);
	return OK;
		
}