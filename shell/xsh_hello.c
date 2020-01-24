/* xsh_hello.c - xsh_hello */

#include <xinu.h>
#include <stdio.h>

/*xsh_hello - write argument to append to hello*/

shellcmd xsh_hello(int nargs, char *args[])
{
	

	if (nargs == 2)
	{
		
		printf("%s%s%s","Hello ",  args[1], ", Welcome to the world of Xinu!!");
		
		printf("\n");
		return 0;

	}
	else if (nargs > 2)
	{
		printf("%s", "Error: More than 1 arguments not allowed");
		printf("\n");
		return -1;
	}
	else if (nargs < 2)
	{
		printf("%s", "Error: Less than 1 argument not allowed");
		printf("\n");
		return -1;
	}
	else
	{
		printf("%s", "Unexepcted Error");
		printf("\n");
		return -1;
	}
}
