/*----------------------------------------------------------------*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_THREADS 4
#define THREAD_1 0
#define THREAD_2 1
#define THREAD_3 2
#define THREAD_4 3
#define ARRAY_SIZE 100

/* POSIX thread declarations and scheduling attributes */
pthread_t threads[NUM_THREADS];

int t1_numPrimes = 0;
int t2_numPrimes = 0;
int t3_numPrimes = 0;
int t4_numPrimes = 0;

typedef struct
{
    int startNum;
    int endNum;
    int *numPrimes;
} threadParams_t;

threadParams_t threadParams[NUM_THREADS];
/*----------------------------------------------------------------*/
int isPrime(int num) {

    return 0;
}
/*----------------------------------------------------------------*/
void *findPrimes(void *threadp) {
    int i = 0;
    int total = 0;
    /* If more time, find smarter way to setup arrays */
    int numberArray[ARRAY_SIZE] = {0};
    int isPrimeArray[ARRAY_SIZE] = {0};

    threadParams_t *threadParams = (threadParams_t *)threadp;

    if(threadParams->endNum < threadParams->startNum){
        printf("invalid input for startNum %d and endNum %d\n", threadParams->endNum, threadParams->startNum);
    }

    /* Setup arrays */
    for(i=0; (i+threadParams->startNum)<threadParams->endNum; i++) {
        numberArray[i] = threadParams->startNum + i;
    }
    for(i=0; i<ARRAY_SIZE; i++) {
        isPrimeArray[i] = 1;
    }

    /* iterate through set of numbers and determine if which values are prime */
    /* NOTE - needed to find a smarter way to interate through this */
    for(i=1; i<ARRAY_SIZE; i+=2) {
        /* confirm divisible, remove from isPrime List */
        if((numberArray[i] % 2) == 0) {
            isPrimeArray[i] = 0;
        }
    }
    for(i=2; i<ARRAY_SIZE; i+=3) {
        /* confirm divisible, remove from isPrime List */
        if((numberArray[i] % 3) == 0) {
            isPrimeArray[i] = 0;
        }
    }
    for(i=4; i<ARRAY_SIZE; i+=5) {
        /* confirm divisible, remove from isPrime List */
        if((numberArray[i] % 5) == 0) {
            isPrimeArray[i] = 0;
        }
    }

    /* Iterate through array, finding all prime numbers */
    for(i=0; i<ARRAY_SIZE; i++) {
        if(isPrimeArray[i] == 1) {
            printf("%d is prime\n", numberArray[i]);
            *(threadParams->numPrimes) += 1;
        }
    }
}


/*----------------------------------------------------------------*/
int main (int argc, char *argv[])
{
   int i = 0;
   int totalPrimes = 0;

   /* Setup threadParams */
   threadParams[THREAD_1].startNum = 1;
   threadParams[THREAD_2].startNum = 101;
   threadParams[THREAD_3].startNum = 201;
   threadParams[THREAD_4].startNum = 301;
   threadParams[THREAD_1].endNum = 100;
   threadParams[THREAD_2].endNum = 200;
   threadParams[THREAD_3].endNum = 300;
   threadParams[THREAD_4].endNum = 400;
   threadParams[THREAD_1].numPrimes = &t1_numPrimes;
   threadParams[THREAD_2].numPrimes = &t2_numPrimes;
   threadParams[THREAD_3].numPrimes = &t3_numPrimes;
   threadParams[THREAD_4].numPrimes = &t4_numPrimes;

    /* Create threads */
   pthread_create(&threads[THREAD_1], NULL, findPrimes, (void *)&threadParams[THREAD_1]);
   /* NOTE - could use the prime numbers of previous thread runs to know which prime values to use */
   /* Could update a list of previous prime numbers to iterate through each set of numbers each thread has */

   pthread_create(&threads[THREAD_2], NULL, findPrimes, (void *)&threadParams[THREAD_2]);
   pthread_create(&threads[THREAD_3], NULL, findPrimes, (void *)&threadParams[THREAD_3]);
   pthread_create(&threads[THREAD_4], NULL, findPrimes, (void *)&threadParams[THREAD_4]);

   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

    totalPrimes = t1_numPrimes + t2_numPrimes + t3_numPrimes + t4_numPrimes;
    printf("Total number of prime values: %ld\n", totalPrimes);

    return 0;
}
/*----------------------------------------------------------------*/