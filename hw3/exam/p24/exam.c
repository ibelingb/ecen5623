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

    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=(threadParams->startNum); i<(threadParams->endNum+1); i++) {
        total += i;
        *(threadParams->numPrimes) += 1;
    }


    printf("Start: %d | End: %d \n", threadParams->startNum, threadParams->endNum);
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