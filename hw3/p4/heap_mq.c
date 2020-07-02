/****************************************************************************/
/*                                                                          */
/* Sam Siewert - 10/14/97                                                   */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ERROR -1

#define NUM_THREADS 2
#define THREAD_1 0
#define THREAD_2 1
#define SCHED_POLICY SCHED_FIFO

#define SNDRCV_MQ "/heap_send_receive_mq"

struct mq_attr mq_attr;
static mqd_t mymq;
static char imagebuff[4096];

/* POSIX thread declarations and scheduling attributes */
pthread_t threads[NUM_THREADS];

struct mq_attr mq_attr;

typedef struct
{
  int param;
} threadParams_t;

threadParams_t threadParams[NUM_THREADS];
struct sched_param schedParam;

/* receives pointer to heap, reads it, and deallocate heap memory */
void *receiver(void *threadp)
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr; 
  int prio;
  int nbytes;
  int count = 0;
  int id;
 
  while(1) {

    /* read oldest, highest priority msg from the message queue */

    printf("Reading %ld bytes\n", sizeof(void *));
  
    if((nbytes = mq_receive(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), &prio)) == ERROR)
/*
    if((nbytes = mq_receive(mymq, (void *)&buffptr, (size_t)sizeof(void *), &prio)) == ERROR)
*/
    {
      perror("mq_receive");
    }
    else
    {
      memcpy(&buffptr, buffer, sizeof(void *));
      memcpy((void *)&id, &(buffer[sizeof(void *)]), sizeof(int));
      printf("receive: ptr msg 0x%X received with priority = %d, length = %d, id = %d\n", buffptr, prio, nbytes, id);

      printf("contents of ptr = \n%s\n", (char *)buffptr);

      free(buffptr);

      printf("heap space memory freed\n");

    }
    
  }

}


void *sender(void *threadp)
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr;
  int prio;
  int nbytes;
  int id = 999;


  while(1) {

    /* send malloc'd message with priority=30 */

    buffptr = (void *)malloc(sizeof(imagebuff));
    strcpy(buffptr, imagebuff);
    printf("Message to send = %s\n", (char *)buffptr);

    printf("Sending %ld bytes\n", sizeof(buffptr));

    memcpy(buffer, &buffptr, sizeof(void *));
    memcpy(&(buffer[sizeof(void *)]), (void *)&id, sizeof(int));

    if((nbytes = mq_send(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), 30)) == ERROR)
    {
      perror("mq_send");
    }
    else
    {
      printf("send: message ptr 0x%X successfully sent\n", buffptr);
    }

    sleep (3);

  }
  
}

void main(void)
{
  int i, j;
  char pixel = 'A';
  int maxPriority;

  /* Cleanup MQs from previous runs */
  mq_unlink(SNDRCV_MQ);
  mq_close(SNDRCV_MQ);

  for (i = 0; i < 4096; i += 64)
  {
    pixel = 'A';
    for (j = i; j < i + 64; j++)
    {
      imagebuff[j] = (char)pixel++;
    }
    imagebuff[j - 1] = '\n';
  }
  imagebuff[4095] = '\0';
  imagebuff[63] = '\0';

  printf("buffer =\n%s", imagebuff);

  // setup common message q attributes
  mq_attr.mq_maxmsg = 100;
  mq_attr.mq_msgsize = sizeof(void *) + sizeof(int);

  mq_attr.mq_flags = 0;

  // note that VxWorks does not deal with permissions?
  mymq = mq_open(SNDRCV_MQ, O_CREAT | O_RDWR, 0, &mq_attr);

  if (mymq == (mqd_t)ERROR)
    perror("mq_open");

  /* Set Sched priority to max value */
  maxPriority = sched_get_priority_max(SCHED_POLICY);
  schedParam.sched_priority = maxPriority;
  sched_setscheduler(getpid(), SCHED_POLICY, &schedParam);

  pthread_create(&threads[THREAD_1], NULL, receiver, (void *)&threadParams[THREAD_1]);
  pthread_create(&threads[THREAD_2], NULL, sender, (void *)&threadParams[THREAD_2]);

  for (i = 0; i < NUM_THREADS; i++)
  {
    pthread_join(threads[i], NULL);
  }

  /* Cleanup MQs */
  mq_unlink(SNDRCV_MQ);
  mq_close(SNDRCV_MQ);
}

/*
static int sid, rid;

void heap_mq(void)
{
  int i, j;
  char pixel = 'A';

  for(i=0;i<4096;i+=64) {
    pixel = 'A';
    for(j=i;j<i+64;j++) {
      imagebuff[j] = (char)pixel++;
    }
    imagebuff[j-1] = '\n';
  }
  imagebuff[4095] = '\0';
  imagebuff[63] = '\0';

  printf("buffer =\n%s", imagebuff);

  // setup common message q attributes
  mq_attr.mq_maxmsg = 100;
  mq_attr.mq_msgsize = sizeof(void *)+sizeof(int);

  mq_attr.mq_flags = 0;

  // note that VxWorks does not deal with permissions?
  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0, &mq_attr);

  if(mymq == (mqd_t)ERROR)
    perror("mq_open");

  // receiver runs at a higher priority than the sender
  if((rid=taskSpawn("Receiver", 90, 0, 4000, receiver, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)) == ERROR) {
    printf("Receiver task spawn failed\n");
  }
  else
    printf("Receiver task spawned\n");

  if((sid=taskSpawn("Sender", 100, 0, 4000, sender, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)) == ERROR) {
    printf("Sender task spawn failed\n");
  }
  else
    printf("Sender task spawned\n");

}
*/