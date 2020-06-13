/* p4_fibTest.c
 * ECEN5623 - Real-Time Embedded Systems
 * Author: Brian Ibeling
 * Date: 6/12/2020
 *
 * Code below seeks to execute POSIX pThreads as Rate Monotonic RTOS tasks in calculating 
 * the Fibonacci Sequence up to a sequence of 10 and 20 numbers with a target deadline of
 * 10 msec and 20 msec for each respsectively.
 * 
 * References and notes used in this code:
 *  - https://computing.llnl.gov/tutorials/pthreads/
 *  - http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/simplethread/pthread.c
 *  - http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/code/RT-Clock/posix_clock.c
 *  - 
 *  - 
 *  - 
*/
/*----------------------------------------------------------------*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#define NUM_THREADS 2
/*----------------------------------------------------------------*/
// POSIX thread declarations and scheduling attributes
typedef struct
{
    int fibSequenceNum;
    int *fibSequenceResult;
    struct timespec *stopClkTime;
} threadParams_t;

pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

static struct timespec rtclk_start_time      = {0, 0};
static struct timespec rtclk_fib10_stop_time = {0, 0};
static struct timespec rtclk_fib20_stop_time = {0, 0};
static int fib10Result = 0;
static int fib20Result = 0;

int fibonacci(int num);

/*----------------------------------------------------------------*/
int fibonacci(int num) {
    int result = 0;

    /* Compute Fibonacci Sequence */
    if(num == 0 || num == 1) {
        result = num;
    } else {
        result = (fibonacci(num-1) + fibonacci(num-2));
    }

    return result;
}
/*----------------------------------------------------------------*/
void fibonacciThread(void *threadp) {
    threadParams_t *threadParams = (threadParams_t *)threadp;
    
    /* Compute Fibonacci Sequence */
    *(threadParams->fibSequenceResult) = fibonacci(threadParams->fibSequenceNum);

    /* Capture computation end time */
    clock_gettime(CLOCK_REALTIME, threadParams->stopClkTime);
}

/*----------------------------------------------------------------*/
int main (int argc, char *argv[])
{
    int i = 0;

   /* Capture time of program start */
   clock_gettime(CLOCK_REALTIME, &rtclk_start_time);

   /* Populate threadParams variable */
   threadParams[0].fibSequenceNum = 10;
   threadParams[0].fibSequenceResult = &fib10Result;
   threadParams[0].stopClkTime = &rtclk_fib10_stop_time;
   threadParams[1].fibSequenceNum = 20;
   threadParams[1].fibSequenceResult = &fib20Result;
   threadParams[1].stopClkTime = &rtclk_fib20_stop_time;

    /* Create POSIX pThreads */
   for(i=0; i<NUM_THREADS; i++) {
       pthread_create(&threads[i],   // pointer to thread descriptor
                      (void *)0,     // use default attributes
                      fibonacciThread,     // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                      );
   }

    /* Synchronize to wait for all thread to complete */
    for(i=0; i<NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    /* Print compute times captured */
    printf("Fib10 Result: %d\nFib10 Compute time: {Seconds = %ld | nanoseconds = %ld}\n", 
            *(threadParams[0].fibSequenceResult),
            (rtclk_fib10_stop_time.tv_sec - rtclk_start_time.tv_sec), 
            (rtclk_fib10_stop_time.tv_nsec - rtclk_start_time.tv_nsec));
    printf("Fib20Result: %d\nFib20 Compute time: {Seconds = %ld | nanoseconds = %ld}\n",  
            *(threadParams[1].fibSequenceResult),
            (rtclk_fib20_stop_time.tv_sec - rtclk_start_time.tv_sec), 
            (rtclk_fib20_stop_time.tv_nsec - rtclk_start_time.tv_nsec));

    printf("TEST COMPLETE\n");
}
/*----------------------------------------------------------------*/