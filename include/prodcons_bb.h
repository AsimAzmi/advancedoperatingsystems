#include <xinu.h>

// declare globally shared array

// declare globally shared semaphores

// declare globally shared read and write indices
extern int read_buffer;
extern int write_buffer;
extern int buffer[];
extern sid32 mutex;
extern sid32 produced;
extern sid32 consumed;
//extern int consume_sem;

// function prototypes
void consumer_bb(int count);
void producer_bb(int count);

