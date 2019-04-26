#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>


#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_MSEC (1000000)
#define NSEC_PER_MICROSEC (1000)
#define DELAY_TICKS (1)
#define ERROR (-1)
#define OK (0)

static struct timespec rtclk_dt1 = {0,0};
static struct timespec rtclk_dt2 = {0,0};
static struct timespec rtclk_start_time = {0,0};
static struct timespec rtclk_stop_time1 = {0,0};
static struct timespec rtclk_stop_time2 = {0,0};
unsigned int idx = 0, jdx = 1;
unsigned int seqIterations = 47;
unsigned int reqIterations = 10000000;
volatile unsigned int fib = 0, fib0 = 0, fib1 = 1;
int abortTest_10, abortTest_20;


#define FIB_TEST(seqCnt, iterCnt)      \
   for(idx=0; idx < iterCnt; idx++)    \
   {                                   \
      fib = fib0 + fib1;               \
      while(jdx < seqCnt)              \
      {                                \
         fib0 = fib1;                  \
         fib1 = fib;                   \
         fib = fib0 + fib1;            \
         jdx++;                        \
      }                                \
   }                                   \

typedef struct
{
    int threadIdx;
} threadParams_t;

pthread_attr_t rt10_sched_attr, rt20_sched_attr,main_sched_attr;
pthread_attr_t main_attr;
pthread_t testThread10, testThread20;
int rt_max_prio, rt_min_prio;
struct sched_param nrt_param;
struct sched_param main_param;
struct sched_param rt10_param;
struct sched_param rt20_param;
pid_t mainpid;
sem_t sem_t10, sem_t20;


int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  if(dt_sec >= 0)
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }
  else
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }

  return(1);
}

void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_OTHER\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }

}

void end_delay_test1()
{
  printf("\n");
  printf("Thread 1 Priority is : %d\n",rt10_param.sched_priority);
  //printf("RT clock start seconds = %ld, nanoseconds = %ld\n", 
         //rtclk_start_time.tv_sec, rtclk_start_time.tv_nsec);

  //printf("RT clock stop seconds = %ld, nanoseconds = %ld\n", 
         //rtclk_stop_time1.tv_sec, rtclk_stop_time1.tv_nsec);

  printf("timestamp = %ld ms (%ld us)\n", 
         (rtclk_dt1.tv_nsec/1000000),(rtclk_dt1.tv_nsec/1000));

 

}

void end_delay_test2()
{
  printf("\n");
  printf("Thread 2 Priority is : %d\n",rt20_param.sched_priority);
  //printf("RT clock start seconds = %ld, nanoseconds = %ld\n", 
        // rtclk_start_time.tv_sec, rtclk_start_time.tv_nsec);

  //printf("RT clock stop seconds = %ld, nanoseconds = %ld\n", 
         //rtclk_stop_time2.tv_sec, rtclk_stop_time2.tv_nsec);

  printf("timestamp = %ld ms (%ld us)\n", 
          (rtclk_dt2.tv_nsec/1000000),(rtclk_dt1.tv_nsec/1000));



}


void *Thread10 (void *threadid)
{
	struct sched_param param;
	//int policy;
	//unsigned long mask = 1;
	threadParams_t *threadParams = (threadParams_t *) threadid;
	while (!abortTest_10)
	{
		sem_wait(&sem_t10);
		/*if(pthread_setaffinity_np(pthread_self(),sizeof(mask),&mask)<0)
		{
			perror("pthread_setaffinity_np");
		}*/
		
		FIB_TEST(seqIterations,6055900);
		clock_gettime(CLOCK_REALTIME, &rtclk_stop_time1);  
		delta_t(&rtclk_stop_time1, &rtclk_start_time, &rtclk_dt1);
		end_delay_test1();
		//pthread_getschedparam(testThread10, &policy, &param);
		
	}
}

void *Thread20 (void *threadid)
{
	struct sched_param param;
	//int policy;
	//unsigned long mask = 1;
	threadParams_t *threadParams = (threadParams_t *) threadid;
	while (!abortTest_20)
	{
		sem_wait(&sem_t20);
		/*if(pthread_setaffinity_np(pthread_self(),sizeof(mask),&mask)<0)
		{
			perror("pthread_setaffinity_np");
		}
		*/
		FIB_TEST(seqIterations,12111420);
		clock_gettime(CLOCK_REALTIME, &rtclk_stop_time2);  
		delta_t(&rtclk_stop_time2, &rtclk_start_time, &rtclk_dt2);
		end_delay_test2();
		//pthread_getschedparam(testThread20, &policy, &param);
	}
}



void main()
{
   int rc, scope;
useconds_t t_10 = 10000;
useconds_t t_20 = 20000;
 
   cpu_set_t threadcpu;
   sem_init (&(sem_t10),0,1);
   sem_init (&(sem_t20),0,1);
   
   	/*if (sem_init (&(sem_t10),0,1))
		{
			printf("Failed to initialize sem_t10");
			exit(-1);
		}
		
	if (sem_init (&(sem_t20),0,1))
		{
			printf("Failed to initialize sem_t20");
			exit(-1);
		}*/
   
   print_scheduler();
   
   pthread_attr_init(&rt10_sched_attr);
   pthread_attr_init(&rt20_sched_attr);
   pthread_attr_setinheritsched(&rt10_sched_attr,PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&rt10_sched_attr,SCHED_FIFO);
   pthread_attr_setinheritsched(&rt20_sched_attr,PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&rt20_sched_attr,SCHED_FIFO);
  
   
   rt_max_prio = sched_get_priority_max(SCHED_FIFO);
   rt_min_prio = sched_get_priority_min(SCHED_FIFO);
   
   mainpid=getpid();
   rc=sched_getparam(mainpid, &main_param);
   main_param.sched_priority = rt_max_prio;
   rt10_param.sched_priority = rt_max_prio-1;
   rt20_param.sched_priority = rt_max_prio-2;
   
   rc = sched_setscheduler(getpid(),SCHED_FIFO, &main_param);
   
   CPU_ZERO(&threadcpu);
   CPU_SET(0,&threadcpu);
   rc=pthread_attr_setaffinity_np(&rt10_sched_attr, sizeof(cpu_set_t), &threadcpu);
   rc=pthread_attr_setaffinity_np(&rt20_sched_attr, sizeof(cpu_set_t), &threadcpu);	
   pthread_attr_setschedparam(&rt10_sched_attr, &rt10_param);
   pthread_attr_setschedparam(&rt20_sched_attr, &rt20_param);
   
   print_scheduler();
   
   pthread_attr_getscope(&main_attr, &scope);
   if(scope == PTHREAD_SCOPE_SYSTEM)
     printf("PTHREAD SCOPE SYSTEM\n");
   else if (scope == PTHREAD_SCOPE_PROCESS)
     printf("PTHREAD SCOPE PROCESS\n");
   else
     printf("PTHREAD SCOPE UNKNOWN\n");
     
   clock_gettime(CLOCK_REALTIME, &rtclk_start_time);    
   rc= pthread_create(&testThread10,&rt10_sched_attr,Thread10,(void *)0);
   if (rc)
	{
		printf("ERROR; pthread_create() rc is %d\n",rc); perror(NULL);
		exit(-1);
	}
	
   rc= pthread_create(&testThread20,&rt20_sched_attr,Thread20,(void *)0);
   if (rc)
	{
		printf("ERROR; pthread_create() rc is %d\n",rc); perror(NULL);
		exit(-1);
	}
	
	usleep(t_20);
	sem_post(&sem_t10);
	usleep(t_20);
	sem_post(&sem_t10);
	usleep(t_10);
	abortTest_20=1;
	sem_post(&sem_t20);
	usleep(t_10);
	sem_post(&sem_t10);
	usleep(t_20);
	abortTest_10=1;
	sem_post(&sem_t10);
	usleep(t_20);
	
	pthread_join (testThread10,NULL);
	pthread_join (testThread20,NULL);
	
	sem_destroy (&(sem_t10));
	sem_destroy (&(sem_t20));
	
}
