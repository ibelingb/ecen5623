/****************************************************************************/
/* Function: Basic POSIX message queue demo from VxWorks Prog. Guide p. 78  */
/*                                                                          */
/* Sam Siewert - 9/24/97                                                    */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ERROR (-1)

#define NUM_THREADS 2
#define HIGH_PRIORITY_THREAD 0
#define LOW_PRIORITY_THREAD 1
#define SCHED_POLICY SCHED_FIFO

#define SNDRCV_MQ "/posix_send_receive_mq"
#define MAX_MSG_SIZE 128

/* POSIX thread declarations and scheduling attributes */
pthread_t threads[NUM_THREADS];

struct mq_attr mq_attr;

typedef struct
{
  int arg1;
  int arg2;
  int arg3;
  int arg4;
  int arg5;
  int arg6;
  int arg7;
  int arg8;
  int arg9;
  int arg10;
} threadParams_t;

threadParams_t threadParams[NUM_THREADS];
pthread_attr_t schedAttr;
struct sched_param schedParam;

/*----------------------------------------------------------------*/
void *receiver(void *threadp)
{
  mqd_t mymq;
  char buffer[MAX_MSG_SIZE];
  int prio;
  int nbytes;

  /* note that VxWorks does not deal with permissions? */
  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, S_IRWXU, &mq_attr);

  if(mymq == (mqd_t)ERROR)
    perror("mq_open");

  /* read oldest, highest priority msg from the message queue */
  if((nbytes = mq_receive(mymq, buffer, MAX_MSG_SIZE, &prio)) == ERROR)
  {
    perror("mq_receive");
  }
  else
  {
    buffer[nbytes] = '\0';
    printf("receive: msg %s received with priority = %d, length = %d\n",
           buffer, prio, nbytes);
  }

  /* Cleanup MQ */
  mq_unlink(SNDRCV_MQ);
  mq_close(mymq);
}

static char canned_msg[] = "this is a test, and only a test, in the event of a real emergency, you would be instructed ...";

/*----------------------------------------------------------------*/
void *sender(void *threadp) 
{
  mqd_t mymq;
  int prio;
  int nbytes;

  /* note that VxWorks does not deal with permissions? */
  mymq = mq_open(SNDRCV_MQ, O_RDWR, S_IRWXU, &mq_attr);

  if(mymq == (mqd_t)ERROR)
    perror("mq_open");

  /* send message with priority=30 */
  if((nbytes = mq_send(mymq, canned_msg, sizeof(canned_msg), 30)) == ERROR)
  {
    perror("mq_send");
  }
  else
  {
    printf("send: message successfully sent\n");
  }
  
  /* Cleanup MQ */
  mq_unlink(SNDRCV_MQ);
  mq_close(mymq);
}

/*----------------------------------------------------------------*/
int main(int argc, char *argv[]) {
  int i = 0;
  int maxPriority, lowPriority, highPriority;

   /* Set Scheduler Policy to FIFO */
   pthread_attr_init(&schedAttr);
   pthread_attr_setinheritsched(&schedAttr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&schedAttr, SCHED_POLICY);

   /* Set Sched priority to max value */
   maxPriority = sched_get_priority_max(SCHED_POLICY);
   schedParam.sched_priority = maxPriority;
   sched_setscheduler(getpid(), SCHED_POLICY, &schedParam);

   // setup common message q attributes
   mq_attr.mq_maxmsg = 100;
   mq_attr.mq_msgsize = MAX_MSG_SIZE;
   mq_attr.mq_flags = 0;

   /* Define priorities */
   lowPriority  = 80;
   highPriority = 90;

   schedParam.sched_priority = highPriority;
   pthread_attr_setschedparam(&schedAttr, &schedParam);
   pthread_create(&threads[HIGH_PRIORITY_THREAD], &schedAttr, receiver, (void *)&threadParams[HIGH_PRIORITY_THREAD]);

   schedParam.sched_priority = lowPriority;
   pthread_attr_setschedparam(&schedAttr, &schedParam);
   pthread_create(&threads[LOW_PRIORITY_THREAD], &schedAttr, sender, (void *)&threadParams[LOW_PRIORITY_THREAD]);

   for (i = 0; i < NUM_THREADS; i++)
     pthread_join(threads[i], NULL);

   return 0;
}
/*----------------------------------------------------------------*/
/* VxWorks Example
void mq_demo(void)
{

  // setup common message q attributes
  mq_attr.mq_maxmsg = 100;
  mq_attr.mq_msgsize = MAX_MSG_SIZE;

  mq_attr.mq_flags = 0;


  // receiver runs at a higher priority than the sender 
  if(taskSpawn("Receiver", 90, 0, 4000, receiver, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR) {
    printf("Receiver task spawn failed\n");
  }
  else
    printf("Receiver task spawned\n");

  if(taskSpawn("Sender", 100, 0, 4000, sender, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR) {
    printf("Sender task spawn failed\n");
  }
  else
    printf("Sender task spawned\n");

   
}
*/