/****************************************************************************************************************************************
/*File Name : Inter-Process Communication using POSIX Message Queue, 
/*Author : Amreeta Sengupta & Vatsal Sheth											
/*Dated 2/28/2019												
/* Code description : One way communicatin is being performed between two threads infinitely. 	  	
/* ************************************************************************************************************************************/

#define _GNU_SOURCE
#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>           
#include <sys/stat.h>        
#include <string.h>
#include <time.h>
#include <sys/syscall.h>
#include <signal.h>

#define queue_name "/send_receive_mq"
#define NUM_THREADS (2)

#define handle_error(msg) \
			{ perror(msg); \
			exit(EXIT_FAILURE); }
                      
mqd_t q_id;
struct mq_attr q_attr;

// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
pthread_attr_t rt_sched_attr[NUM_THREADS];
int rt_max_prio;
struct sched_param rt_param[NUM_THREADS];
struct sched_param main_param;
pthread_attr_t main_attr;
pid_t mainpid;

static char imagebuff[4096];

void print_scheduler(void);
void *sender(void *);
void *receiver(void *);

int main()
{
	int rc;
	cpu_set_t cpuset;
	int i, j;
	char pixel = 'A';

	for(i=0;i<4096;i+=64) 
	{
		pixel = 'A';
		for(j=i;j<i+64;j++) 
		{
			imagebuff[j] = (char)pixel++;
		}
		imagebuff[j-1] = '\n';
	}
	imagebuff[4095] = '\0';
	imagebuff[63] = '\0';

	printf("buffer =\n%s", imagebuff);
  
	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);
	
	mainpid=getpid();

	rt_max_prio = sched_get_priority_max(SCHED_FIFO);
	
	print_scheduler();	
	rc = sched_getparam(mainpid, &main_param);
	main_param.sched_priority=rt_max_prio;
	rc = sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
	if(rc < 0) 
		handle_error("main_param");
   
	print_scheduler();
   
	q_attr.mq_flags = 0;
    q_attr.mq_maxmsg = 100;
    q_attr.mq_msgsize = sizeof(void *)+sizeof(int);
    q_attr.mq_curmsgs = 0;
	
	q_id = mq_open(queue_name, O_RDWR | O_CREAT, 0664, &q_attr);
	if(q_id == -1)
		handle_error("mq_open")
	
	rc = pthread_attr_init(&rt_sched_attr[0]);
    rc = pthread_attr_setinheritsched(&rt_sched_attr[0], PTHREAD_EXPLICIT_SCHED);
    rc = pthread_attr_setschedpolicy(&rt_sched_attr[0], SCHED_FIFO);
    rc = pthread_attr_setaffinity_np(&rt_sched_attr[0], sizeof(cpu_set_t), &cpuset);

    rt_param[0].sched_priority = rt_max_prio-1;
    pthread_attr_setschedparam(&rt_sched_attr[0], &rt_param[0]);
    
    rc = pthread_attr_init(&rt_sched_attr[1]);
    rc = pthread_attr_setinheritsched(&rt_sched_attr[1], PTHREAD_EXPLICIT_SCHED);
    rc = pthread_attr_setschedpolicy(&rt_sched_attr[1], SCHED_FIFO);
    rc = pthread_attr_setaffinity_np(&rt_sched_attr[1], sizeof(cpu_set_t), &cpuset);

    rt_param[1].sched_priority = rt_max_prio-2;
    pthread_attr_setschedparam(&rt_sched_attr[1], &rt_param[1]);

    rc = pthread_create(&threads[0], (void *)&rt_sched_attr, receiver, (void *)0);
    if(rc != 0)
		handle_error("pthread create");
		
	rc = pthread_create(&threads[1], (void *)&rt_sched_attr, sender, (void *)0);
    if(rc != 0)
		handle_error("pthread create");
	
	rc = pthread_join(threads[0], NULL);
	if(rc != 0)
		handle_error("pthread join")
		
	rc = pthread_join(threads[1], NULL);
	if(rc != 0)
		handle_error("pthread join");
		
	mq_close(q_id);
	if(rc == -1)
		handle_error("mq_close");
	
	return 0;
}

void *receiver(void * a)
{
	char buffer[sizeof(void *)+sizeof(int)];
	void *buffptr; 
	int prio;
	int count = 0;
	int id, rc;
 
	while(1) 
	{
		/* read oldest, highest priority msg from the message queue */
		printf("Reading %ld bytes\n", (sizeof(void *)+sizeof(int)));
		
		rc = mq_receive(q_id, buffer, (size_t)(sizeof(void *)+sizeof(int)), &prio);
		if(rc == -1)
			handle_error("mq_receive");
			
        memcpy(&buffptr, buffer, sizeof(void *));
		memcpy((void *)&id, &(buffer[sizeof(void *)]), sizeof(int));
		printf("receive: ptr msg 0x%p received with priority = %d, length = %d, id = %d\n", buffptr, prio, rc, id);

		printf("contents of ptr = \n%s\n", (char *)buffptr);	

		free(buffptr);

		printf("heap space memory freed\n");    
	}
}

void *sender(void *a)
{
	char buffer[sizeof(void *)+sizeof(int)];
	void *buffptr;
	int rc;
	int id = 999;
	
	while(1) 
	{
		/* send malloc'd message with priority=30 */

		buffptr = (void *)malloc(sizeof(imagebuff));
		strcpy(buffptr, imagebuff);
		printf("Message to send = %s\n", (char *)buffptr);

		printf("Sending %ld bytes\n", (sizeof(void *)+sizeof(int)));

		memcpy(buffer, &buffptr, sizeof(void *));
		memcpy(&(buffer[sizeof(void *)]), (void *)&id, sizeof(int));

		rc = mq_send(q_id, (const char *)&buffer, (size_t)(sizeof(void *)+sizeof(int)), 30); 
		if(rc == -1)
			handle_error("mq_send");
    
        printf("send: message ptr 0x%p successfully sent\n", buffptr);
        usleep(10000);
	}
}

void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("\nPthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("\nPthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("\nPthread Policy is SCHED_OTHER\n");
           break;
     default:
       printf("\nPthread Policy is UNKNOWN\n");
   }

}
