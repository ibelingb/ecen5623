/* p5_mutexDatTimeout.c
 * ECEN5623 - Real-Time Embedded Systems
 * Author: Brian Ibeling
 * Date: 6/29/2020
 *
 * Code to demonstrate the use of Mutex timeout with a pThread waiting for a signal from 
 * a mutex semaphore. If unable to lock to mutex in a 10 second window, thread unblocks
 * and prints a timeout message to the user.
 * 
 * References and notes used in this code:
 *  - https://computing.llnl.gov/tutorials/pthreads/
 *  - http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/simplethread/pthread.c
 *  - https://linux.die.net/man/3/pthread_mutex_lock
 *  - http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/code/example-sync-updated-2/pthread3.c
 *  - https://linux.die.net/man/3/pthread_mutex_timedlock
*/
/*----------------------------------------------------------------*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_THREADS 1
#define LOW_PRIO_SERVICE 0
#define HIGH_PRIO_SERVICE 1
#define SCHED_POLICY SCHED_FIFO
#define WRITE_THREAD_SEC_TIME 3
#define WRITE_THREAD_NSEC_TIME 0
#define READ_THREAD_SEC_TIME 5
#define READ_THREAD_NSEC_TIME 0

/* POSIX thread declarations and scheduling attributes */
pthread_t threads[NUM_THREADS];
pthread_mutex_t sharedMemMutex;
pthread_mutex_t newDataMutex;
pthread_attr_t schedAttr;
struct sched_param schedParam;
int rtPrioLow;
int rtPrioHigh;

/* Setup global data struct and thread sample rates */
typedef struct {
    struct timespec timeSampled;
    double accel_x;
    double accel_y;
    double accel_z;
    double roll;
    double pitch;
    double yaw;
} navData_t;

navData_t globalData = {0};
static struct timespec write_thread_sleep_time = {WRITE_THREAD_SEC_TIME, WRITE_THREAD_NSEC_TIME};
static struct timespec read_thread_timeout = {READ_THREAD_SEC_TIME, READ_THREAD_NSEC_TIME};

/*----------------------------------------------------------------*/
/* Thread function which continuously updates global data, locking and unlocking the shared mutex
 * immediately before and after each write event. Thread loops and sleeps on set WRITE_THREAD_SEC_TIME 
 * and WRITE_THREAD_NSEC_TIME time internal.
 */
void *updatePositionAttitudeState()
{
    while (1) 
    {

        /* Lock/Unlock critical section before/after updating global data */
        /* Update global data */
        pthread_mutex_lock(&sharedMemMutex);
        clock_gettime(CLOCK_REALTIME, &globalData.timeSampled);
        globalData.accel_x += 0.1;
        globalData.accel_y += 0.2;
        globalData.accel_z += 0.3;
        globalData.roll    += 0.3;
        globalData.pitch   += 0.2;
        globalData.yaw     += 0.1;
        pthread_mutex_unlock(&sharedMemMutex);

        /* New Data available, unlock newDataMutex to signal ReadThread */
        pthread_mutex_lock(&newDataMutex);
        
        // TODO - delay needed here?

        /* Lock the new data mutex while waiting for the next data sample to occur */
        pthread_mutex_lock(&newDataMutex);
        nanosleep(&write_thread_sleep_time, &write_thread_sleep_time);
    }
}
/*----------------------------------------------------------------*/
/* Thread function which continuously reads global data, locking and unlocking the shared mutex
 * immediately before and after each read event. Thread waits on a semaphore mutex and if data 
 * no received within (READ_THREAD_SEC_TIME + READ_THREAD_NSEC_TIME) time internal, a timeout 
 * message is printed and the thread loops again.
 */
void *readPositionAttitudeState() {
    navData_t localData = {0};
    int newDataAvailable = 0;
    //struct timespec timeSampled;

    while (1)
    {
        /* Lock/Unlock critical section before/after read from global data */
        /* Copy global data into local data struct */
        newDataAvailable = pthread_mutex_timedlock(&sharedMemMutex, &read_thread_timeout);

        if(newDataAvailable == 0) {
            pthread_mutex_lock(&sharedMemMutex);
            memcpy(&localData, &globalData, sizeof(navData_t));
            pthread_mutex_unlock(&sharedMemMutex);
            /* Print data copied from global data struct */
            printf("Reporting Position and Attitude data:\n \
                    Time: %ld, %ld\n \
                    Accel_X: %f\n \
                    Accel_Y: %f\n \
                    Accel_Z: %f\n \
                    Roll: %f\n \
                    Pitch: %f\n \
                    Yaw: %f\n",
                    localData.timeSampled.tv_sec,
                    localData.timeSampled.tv_nsec,
                    localData.accel_x,
                    localData.accel_y,
                    localData.accel_z,
                    localData.roll,
                    localData.pitch,
                    localData.yaw);
        } else {
            //clock_gettime(CLOCK_REALTIME, &globalData.timeSampled);
            printf("No new data available at time: %ld sec, %ld nsec\n", localData.timeSampled.tv_sec, localData.timeSampled.tv_sec);
        }
    }
}

/*----------------------------------------------------------------*/
/* Set pthread scheduling priorities and policy. This should be called before
 * any threads are created.
 */
void setSchedPolicyPriority() {
    int maxPriority = 0;

    /* Set Scheduler Policy to FIFO */
    pthread_attr_init(&schedAttr);
    pthread_attr_setinheritsched(&schedAttr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&schedAttr, SCHED_POLICY);

    /* Set Sched priority to max value */
    maxPriority = sched_get_priority_max(SCHED_POLICY);
    schedParam.sched_priority = maxPriority;
    sched_setscheduler(getpid(), SCHED_POLICY, &schedParam);
}


/*----------------------------------------------------------------*/
/* Main - Create global data write and read POSIX threads, setting thread priorities and initializing
 * the shared global mutex prior to each thread being created.
 */
int main (int argc, char *argv[])
{
   int i;

   /* Capture thread prio levels */
   rtPrioLow = sched_get_priority_min(SCHED_POLICY) + 1;
   rtPrioHigh = sched_get_priority_min(SCHED_POLICY) + 2;

   pthread_mutex_init(&sharedMemMutex, NULL);

    /* Create threads, setting priority of Nav data write thread higher than the data read thread */
   schedParam.sched_priority = rtPrioLow;
   pthread_attr_setschedparam(&schedAttr, &schedParam);
   pthread_create(&threads[LOW_PRIO_SERVICE], &schedAttr, readPositionAttitudeState, (void *)NULL);

   schedParam.sched_priority = rtPrioHigh;
   pthread_attr_setschedparam(&schedAttr, &schedParam);
   pthread_create(&threads[HIGH_PRIO_SERVICE], &schedAttr, updatePositionAttitudeState, (void *)NULL);

   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&sharedMemMutex);

    return 0;
}
/*----------------------------------------------------------------*/