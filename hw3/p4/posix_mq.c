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
#include <sys/time.h>
#include <mqueue.h>

#define ERROR -1

#define NUM_THREADS 2
#define THREAD_1 0
#define THREAD_2 1

#define SNDRCV_MQ "posix_send_receive_mq"
#define MAX_MSG_SIZE 128

/* POSIX thread declarations and scheduling attributes */
pthread_t threads[NUM_THREADS];

struct mq_attr mq_attr;

typedef struct
{
  int param;
} threadParams_t;

threadParams_t threadParams[NUM_THREADS];

void *receiver(void *threadp)
{
  mqd_t mymq;
  char buffer[MAX_MSG_SIZE];
  int prio;
  int nbytes;

  /* note that VxWorks does not deal with permissions? */
  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0, &mq_attr);

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
    
}

static char canned_msg[] = "this is a test, and only a test, in the event of a real emergency, you would be instructed ...";

void *sender(void *threadp) 
{
  mqd_t mymq;
  int prio;
  int nbytes;

  /* note that VxWorks does not deal with permissions? */
  mymq = mq_open(SNDRCV_MQ, O_RDWR, 0, &mq_attr);

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
  
}

int main(int argc, char *argv[]) {
  int i = 0;

  // setup common message q attributes
  mq_attr.mq_maxmsg = 100;
  mq_attr.mq_msgsize = MAX_MSG_SIZE;

  mq_attr.mq_flags = 0;

  pthread_create(&threads[THREAD_1], NULL, receiver, (void *)&threadParams[THREAD_1]);
  pthread_create(&threads[THREAD_2], NULL, sender, (void *)&threadParams[THREAD_2]);

  for (i = 0; i < NUM_THREADS; i++)
    pthread_join(threads[i], NULL);

  return 0;
}

/*
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