/*----------------------------------------------------------------*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_THREADS 3
#define THREAD_1 0
#define THREAD_2 1
#define THREAD_3 2

/* POSIX thread declarations and scheduling attributes */
pthread_t threads[NUM_THREADS];

int t1_sum = 0;
int t2_sum = 0;
int t3_sum = 0;

typedef struct
{
    int startNum;
    int endNum;
    int *sum;
} threadParams_t;

threadParams_t threadParams[NUM_THREADS];
/*----------------------------------------------------------------*/
void *sumValues(void *threadp) {
    int i = 0;
    int total = 0;

    threadParams_t *threadParams = (threadParams_t *)threadp;
    printf("Start: %d | End: %d\n", threadParams->startNum, threadParams->endNum);

    for(i=(threadParams->startNum); i<(threadParams->endNum+1); i++) {
        total += i;
    }

    *(threadParams->sum) = total;
}


/*----------------------------------------------------------------*/
int main (int argc, char *argv[])
{
   int i = 0;
   int totalSummed = 0;

   /* Setup threadParams */
   threadParams[THREAD_1].startNum = 1;
   threadParams[THREAD_2].startNum = 100;
   threadParams[THREAD_3].startNum = 200;
   threadParams[THREAD_1].endNum = 99;
   threadParams[THREAD_2].endNum = 199;
   threadParams[THREAD_3].endNum = 299;
   threadParams[THREAD_1].sum = &t1_sum;
   threadParams[THREAD_2].sum = &t2_sum;
   threadParams[THREAD_3].sum = &t3_sum;

    /* Create threads */
   pthread_create(&threads[THREAD_1], NULL, sumValues, (void *)&threadParams[THREAD_1]);
   pthread_create(&threads[THREAD_2], NULL, sumValues, (void *)&threadParams[THREAD_2]);
   pthread_create(&threads[THREAD_3], NULL, sumValues, (void *)&threadParams[THREAD_3]);

   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

    totalSummed = t1_sum + t2_sum + t3_sum;
    printf("Total sum of all values: %ld\n", totalSummed);

    return 0;
}
/*----------------------------------------------------------------*/