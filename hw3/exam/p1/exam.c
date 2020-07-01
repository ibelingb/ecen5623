/*----------------------------------------------------------------*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_THREADS 3
#define HIGH_PRIORITY_THREAD 0
#define MED_PRIORITY_THREAD 1
#define LOW_PRIORITY_THREAD 2
#define SCHED_POLICY SCHED_FIFO

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
    int threadNum;
} threadParams_t;

threadParams_t threadParams[NUM_THREADS];
pthread_attr_t schedAttr;
struct sched_param schedParam;
/*----------------------------------------------------------------*/
void *sumValues(void *threadp) {
    int i = 0;
    int total = 0;

    threadParams_t *threadParams = (threadParams_t *)threadp;
    for(i=(threadParams->startNum); i<(threadParams->endNum+1); i++) {
        total += i;
    }

    *(threadParams->sum) = total;

    printf("Thread %d Result: %d\n", threadParams->threadNum, total);
}


/*----------------------------------------------------------------*/
int main (int argc, char *argv[])
{
   int i = 0;
   int totalSummed = 0;
   int maxPriority, minPriority, lowPriority, medPriority, highPriority;

   /* Set Scheduler Policy to FIFO */
   pthread_attr_init(&schedAttr);
   pthread_attr_setinheritsched(&schedAttr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&schedAttr, SCHED_POLICY);

   /* Set Sched priority to max value */
   maxPriority = sched_get_priority_max(SCHED_POLICY);
   schedParam.sched_priority = maxPriority;
   sched_setscheduler(getpid(), SCHED_POLICY, &schedParam);

   /* Capture thread Priorities */
   minPriority = sched_get_priority_min(SCHED_POLICY);
   lowPriority  = minPriority + 1;
   medPriority  = minPriority + 2;
   highPriority = minPriority + 3;

   /* Setup threadParams */
   threadParams[HIGH_PRIORITY_THREAD].startNum = 1;
   threadParams[MED_PRIORITY_THREAD].startNum = 100;
   threadParams[LOW_PRIORITY_THREAD].startNum = 200;
   threadParams[HIGH_PRIORITY_THREAD].endNum = 99;
   threadParams[MED_PRIORITY_THREAD].endNum = 199;
   threadParams[LOW_PRIORITY_THREAD].endNum = 299;
   threadParams[HIGH_PRIORITY_THREAD].sum = &t1_sum;
   threadParams[MED_PRIORITY_THREAD].sum = &t2_sum;
   threadParams[LOW_PRIORITY_THREAD].sum = &t3_sum;
   threadParams[HIGH_PRIORITY_THREAD].threadNum = HIGH_PRIORITY_THREAD;
   threadParams[MED_PRIORITY_THREAD].threadNum = MED_PRIORITY_THREAD;
   threadParams[LOW_PRIORITY_THREAD].threadNum = LOW_PRIORITY_THREAD;

    /* Create threads, setting priority for each */
   
   schedParam.sched_priority = highPriority;
   pthread_attr_setschedparam(&schedAttr, &schedParam);
   pthread_create(&threads[HIGH_PRIORITY_THREAD], &schedAttr, sumValues, (void *)&threadParams[HIGH_PRIORITY_THREAD]);

   schedParam.sched_priority = medPriority;
   pthread_attr_setschedparam(&schedAttr, &schedParam);
   pthread_create(&threads[MED_PRIORITY_THREAD], &schedAttr, sumValues, (void *)&threadParams[MED_PRIORITY_THREAD]);

   schedParam.sched_priority = lowPriority;
   pthread_attr_setschedparam(&schedAttr, &schedParam);
   pthread_create(&threads[LOW_PRIORITY_THREAD], &schedAttr, sumValues, (void *)&threadParams[LOW_PRIORITY_THREAD]);
   
   /* Join all threads to sync once all have completed */
   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

    /* Calculate sum of all threads and print */
    totalSummed = t1_sum + t2_sum + t3_sum;
    printf("Sum total of all threads: %ld\n", totalSummed);

    return 0;
}
/*----------------------------------------------------------------*/