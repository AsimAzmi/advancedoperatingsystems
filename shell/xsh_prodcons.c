/* xsh_prodcons.c - xsh_prodcons */

#include <xinu.h>
#include <prodcons.h>
#include <stdlib.h>

//Definition for global variable 'n'
int n;


shellcmd xsh_prodcons(int nargs, char *args[])
{
       	//Argument verifications and validations
	if ( nargs > 2)
        {
	   printf("%s","Error: Optional argument. Maximum 1 argument allowed");	
	   return -1;
	}		
	
	int count = 2000;             //local varible to hold count
  
 	 //check args[1] if present assign value to count
  	if ( nargs == 2 && args[1] != NULL)
	{
		count = atoi(args[1]);
	}	
	else
	{
		printf("%s", "Error : Argument cannot be null");
	}
	
	// checking if input value is actually a number or not
	if ( count == 0 && args[1] != '0')
	{
		printf("%s", "Error: Only integers allowed");
	}

	printf("Count : %d", count);
	 //create the process producer and consumer and put them in ready queue.
 	//Look at the definations of function create and resume in the system folder for reference.      
 	resume( create(producer, 1024, 20, "producer", 1, count));
 	resume( create(consumer, 1024, 20, "consumer", 1, count));
	return (0);
}
