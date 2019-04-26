 /****************************************************************************************************************************************
/*File Name : Inter-Process Communication using POSIX Message Queue, 
/*Author : Amreeta Sengupta & Vatsal Sheth											
/*Dated 3/07/2019												
/* Code description : One way communicatin is being performed between two threads once. 		  	
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

#define queue_name "/send_receive_mq_test"
#define NUM_THREADS (2)
#define MAX_MSG_SIZE 128
	
#define handle_error(msg) \
			{ perror(msg); \
			exit(EXIT_FAILURE); }
                      

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

static char canned_msg[] = "this is a test, and only a test, in the event of a real emergency, you would be instructed ...";

void print_scheduler(void);
void *sender(void *);
void *receiver(void *);

int main()
{
	int rc;
	cpu_set_t cpuset;
	  
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
    q_attr.mq_msgsize = MAX_MSG_SIZE;
    q_attr.mq_curmsgs = 0;
	
	
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
		
	
	
	return 0;
}

void *receiver(void * a)
{
	char buffer[MAX_MSG_SIZE];
	int prio;
	mqd_t q_id;
	int nbytes, rc; 
	
	q_id = mq_open(queue_name, O_RDWR | O_CREAT, 0664, &q_attr);
	if(q_id == -1)
		handle_error("mq_open")
		
	rc = mq_receive(q_id, buffer, MAX_MSG_SIZE, &prio);
	if(rc == -1)
		handle_error("mq_receive");
			
	buffer[rc] = '\0';
    printf("receive: msg %s received with priority = %d, length = %d\n",buffer, prio, rc); 
    
    mq_close(q_id);
	if(rc == -1)
		handle_error("mq_close");
	
}

void *sender(void *a)
{
	int rc;
	mqd_t q_id;
	
	q_id = mq_open(queue_name, O_RDWR, 0664, &q_attr);
	if(q_id == -1)
		handle_error("mq_open")
		
	rc = mq_send(q_id,(const char *)canned_msg, sizeof(canned_msg), 30); 
	if(rc == -1)
		handle_error("mq_send");
    
    printf("send: message successfully sent\n");
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

