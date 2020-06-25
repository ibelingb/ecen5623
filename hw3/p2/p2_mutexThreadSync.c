/* p2_mutexThreadSync.c
 * ECEN5623 - Real-Time Embedded Systems
 * Author: Brian Ibeling
 * Date: 6/24/2020
 *
 * TODO
 * 
 * References and notes used in this code:
 *  - https://computing.llnl.gov/tutorials/pthreads/
 *  - http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/simplethread/pthread.c
 *  - https://linux.die.net/man/3/pthread_mutex_lock
*/
/*----------------------------------------------------------------*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>

#include <sys/time.h>

#define NUM_THREADS 2
#define LOW_PRIO_SERVICE 0
#define HIGH_PRIO_SERVICE 1
#define SCHED_POLICY SCHED_FIFO

// POSIX thread declarations and scheduling attributes
pthread_t threads[NUM_THREADS];
pthread_attr_t schedAttr;
struct sched_param schedParam;
pthread_mutex_t sharedMemSem;
int rt_max_prio;
int rt_min_prio;

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
static struct timespec sleep_time = {1, 0};

/*----------------------------------------------------------------*/
void *updatePositionAttitudeState()
{
    while (1) {
        pthread_mutex_lock(&sharedMemSem);

        gettimeofday(&globalData.timeSampled, (void *)0);
        globalData.accel_x += 0.1;
        globalData.accel_y += 0.2;
        globalData.accel_z += 0.3;
        globalData.roll    += 0.3;
        globalData.pitch   += 0.2;
        globalData.yaw     += 0.1;

        pthread_mutex_unlock(&sharedMemSem);

        nanosleep(&sleep_time, &sleep_time);
    }
}
/*----------------------------------------------------------------*/
void *readPositionAttitudeState() {
    navData_t localData = {0};

    while (1)
    {
        /* Copy global data into local data struct */
        pthread_mutex_lock(&sharedMemSem);
        memcpy(&localData, &globalData, sizeof(navData_t));
        pthread_mutex_unlock(&sharedMemSem);

        /* Print data copied from global data struct */
        printf("Reporting position data:\n \
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

        nanosleep(&sleep_time, &sleep_time);
    }
}

/*----------------------------------------------------------------*/
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
int main (int argc, char *argv[])
{
   int rc;
   int i;

   rt_max_prio = sched_get_priority_max(SCHED_POLICY);
   rt_min_prio = sched_get_priority_min(SCHED_POLICY);

   pthread_mutex_init(&sharedMemSem, NULL);

   pthread_create(&threads[LOW_PRIO_SERVICE], &schedAttr, readPositionAttitudeState, (void *)NULL);

   pthread_create(&threads[HIGH_PRIO_SERVICE], &schedAttr, updatePositionAttitudeState, (void *)NULL);

   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

}

/*----------------------------------------------------------------*/