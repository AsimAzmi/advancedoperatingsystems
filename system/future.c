#include<future.h>
#include<xinu.h>
#include<stdio.h>
#include<stdlib.h>



// nelems i.e number of elements will always be 1 for now. Its not used
future_t* future_alloc(future_mode_t mode, uint size, uint nelems)
{
	char* future_struct_addr = (char*) getmem(size);
	if (SYSERR == future_struct_addr)
	{
		printf("\n error returned by future_alloc");
		return SYSERR;
	}
	else
	{
		printf("\n memory allocated by future_alloc");
		return future_struct_addr;
	}

}


syscall future_free(future_t* f)
{
	int error = 0;
	error = freemem(f, sizeof(future_t));
	f = NULL;
	if (SYSERR == error)
	{
		printf("\n Error returned by future_free");
		return SYSERR;
	}
	else
	{
		printf("\n memory freed future_free");
		return OK;
	}
		
}