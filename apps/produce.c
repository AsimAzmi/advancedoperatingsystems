#include <xinu.h>
#include <prodcons.h>

void producer(int count) {

	int32 i;

	for ( i = 1; i <= count; i++)
	{
		n = i;
		printf("\n Produced : %d \n", n);
	}

}



