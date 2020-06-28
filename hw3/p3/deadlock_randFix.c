/* deadlock_randFix.c
 * ECEN5623 - Real-Time Embedded Systems
 * Author: Brian Ibeling
 * Date: 6/27/2020
 *
 * Code below executes the deadlock.c but with an added check if deadlock has occurred, and if
 * true to restart both child threads with a randomized delay at the start of each.
 * 
 * Resources and References:
 *  - http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/code/example-sync-updated-2/deadlock.c
 *  - https://linux.die.net/man/3/pthread_exitA
 *  - https://www.tutorialspoint.com/c_standard_library/c_function_rand.htm
 */

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_THREADS 2
#define THREAD_1 1
#define THREAD_2 2

typedef struct
{
    int threadIdx;
    int delaySec;
} threadParams_t;


pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

struct sched_param nrt_param;

// On the Raspberry Pi, the MUTEX semaphores must be statically initialized
//
// This works on all Linux platforms, but dynamic initialization does not work
// on the R-Pi in particular as of June 2020.
//
pthread_mutex_t rsrcA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rsrcB = PTHREAD_MUTEX_INITIALIZER;

volatile int rsrcACnt=0, rsrcBCnt=0, noWait=0;

// Variables to check for deadlock event
int deadlockCheckLoopCount = 0;
int deadlockCheckMaxCount = 10;
int thread1Complete = 0;
int thread2Complete = 0;
int deadlockFlag = 0;

void *grabRsrcs(void *threadp)
{
   threadParams_t *threadParams = (threadParams_t *)threadp;
   int threadIdx = threadParams->threadIdx;

   /* Delay based on randomized input value */
   sleep(threadParams->delaySec);

   if(threadIdx == THREAD_1)
   {
     printf("THREAD 1 grabbing resources\n");
     pthread_mutex_lock(&rsrcA);
     rsrcACnt++;
     if(!noWait) sleep(1);
     printf("THREAD 1 got A, trying for B\n");
     pthread_mutex_lock(&rsrcB);

    /* If deadlockFlag set, exit this thread */
    if(deadlockFlag) {
      rsrcACnt--;
      printf("THREAD 1 Deadlocked, exiting\n");
      pthread_exit(NULL);
    }

     rsrcBCnt++;
     printf("THREAD 1 got A and B\n");
     pthread_mutex_unlock(&rsrcB);
     pthread_mutex_unlock(&rsrcA);
     printf("THREAD 1 done\n");
     thread1Complete = 1;
   }
   else
   {
     printf("THREAD 2 grabbing resources\n");
     pthread_mutex_lock(&rsrcB);
     rsrcBCnt++;
     if(!noWait) sleep(1);
     printf("THREAD 2 got B, trying for A\n");
     pthread_mutex_lock(&rsrcA);

    /* If deadlockFlag set, exit this thread */
    if(deadlockFlag) {
      rsrcBCnt--;
      printf("THREAD 2 Deadlocked, exiting\n");
      pthread_exit(NULL);
    }

     rsrcACnt++;
     printf("THREAD 2 got B and A\n");
     pthread_mutex_unlock(&rsrcA);
     pthread_mutex_unlock(&rsrcB);
     printf("THREAD 2 done\n");
     thread2Complete = 1;
   }
   pthread_exit(NULL);
}


int main (int argc, char *argv[])
{
   int rc, safe=0;

   rsrcACnt=0, rsrcBCnt=0, noWait=0;

   if(argc < 2)
   {
     printf("Will set up unsafe deadlock scenario\n");
   }
   else if(argc == 2)
   {
     if(strncmp("safe", argv[1], 4) == 0)
       safe=1;
     else if(strncmp("race", argv[1], 4) == 0)
       noWait=1;
     else
       printf("Will set up unsafe deadlock scenario\n");
   }
   else
   {
     printf("Usage: deadlock [safe|race|unsafe]\n");
   }


   printf("Creating thread %d\n", THREAD_1);
   threadParams[THREAD_1].threadIdx=THREAD_1;
   threadParams[THREAD_1].delaySec=0;
   rc = pthread_create(&threads[0], NULL, grabRsrcs, (void *)&threadParams[THREAD_1]);
   if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}
   printf("Thread 1 spawned\n");

   if(safe) // Make sure Thread 1 finishes with both resources first
   {
     if(pthread_join(threads[0], NULL) == 0)
       printf("Thread 1: %x done\n", (unsigned int)threads[0]);
     else
       perror("Thread 1");
   }

   printf("Creating thread %d\n", THREAD_2);
   threadParams[THREAD_2].threadIdx=THREAD_2;
   threadParams[THREAD_2].delaySec=0;
   rc = pthread_create(&threads[1], NULL, grabRsrcs, (void *)&threadParams[THREAD_2]);
   if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}
   printf("Thread 2 spawned\n");

   printf("rsrcACnt=%d, rsrcBCnt=%d\n", rsrcACnt, rsrcBCnt);
   printf("will try to join CS threads unless they deadlock\n");

   /* 
    * Monitor if deadlock has occurred by checking to see if both threads execute for an extended period of time
    * without either completing. If deadlock detected, set deadlock Flag, unlock mutexes, and restart
    * with a random delay before each thread takes their first mutex.
    * 
    * A better implementation of this would be run this logic in a parent thread which creates the 2 threads below.
    * This simple implmentation will work for this example demonstration.
    */
    do {
      /* Deadlock detected, cancel both threads and re-create with random delay added to start of both */
      if (deadlockCheckLoopCount >= deadlockCheckMaxCount) {
        printf("ERROR: Deadlock detected!\n");
        /* Set global deadlock flag */
        deadlockFlag = 1;

        /* Unlock both mutexes to allow both threads to exit with deadlockFlag set*/
        pthread_mutex_unlock(&rsrcB);
        pthread_mutex_unlock(&rsrcA);

        /* Wait for both threads to exit, then clear deadlock flag */
        pthread_join(threads[0], NULL);
        pthread_join(threads[1], NULL);
        deadlockFlag = 0;

        /* Release previously held locks (first mutex taken by each thread) */
        pthread_mutex_unlock(&rsrcB);
        pthread_mutex_unlock(&rsrcA);

        /* Calculate random delay values */
        srand(time(NULL));
        threadParams[THREAD_1].delaySec = (rand() % 5);
        sleep(1); /* Delay to ensure random value generated */
        srand(time(NULL));
        threadParams[THREAD_2].delaySec = (rand() % 5);
        printf("Random delay for Thread1: %d\n", threadParams[THREAD_1].delaySec);
        printf("Random delay for Thread2: %d\n", threadParams[THREAD_2].delaySec);

        /* Re-start both threads */
        printf("Thread 1 restart with delay %d seconds\n", threadParams[THREAD_1].delaySec);
        rc = pthread_create(&threads[0], NULL, grabRsrcs, (void *)&threadParams[THREAD_1]);
        if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}

        printf("Thread 2 restart\n");
        printf("Thread 2 restart with delay %d seconds\n", threadParams[THREAD_2].delaySec);
        rc = pthread_create(&threads[1], NULL, grabRsrcs, (void *)&threadParams[THREAD_2]);
        if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}

        deadlockCheckLoopCount = 0;
      }

      printf("Checking Deadlock\n");
      deadlockCheckLoopCount++;
      sleep(1);
    } while((thread1Complete == 0) && (thread2Complete == 0));

    printf("DONE Checking Deadlock\n");

   if(!safe)
   {
     if(pthread_join(threads[0], NULL) == 0)
       printf("Thread 1: %x done\n", (unsigned int)threads[0]);
     else
       perror("Thread 1");
   }

   if(pthread_join(threads[1], NULL) == 0)
     printf("Thread 2: %x done\n", (unsigned int)threads[1]);
   else
     perror("Thread 2");

   if(pthread_mutex_destroy(&rsrcA) != 0)
     perror("mutex A destroy");

   if(pthread_mutex_destroy(&rsrcB) != 0)
     perror("mutex B destroy");

   printf("All done\n");

   exit(0);
}