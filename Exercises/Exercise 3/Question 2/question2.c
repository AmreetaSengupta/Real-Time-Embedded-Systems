/****************************************************************************************************************************************
/*File Name : question2.c
/*Author : Amreeta Sengupta & Vatsal Sheth 											
/*Dated 3/5/2019												
/* Code description : Demonstrate the use of mutex in mutli-threaded application				  	
/* ************************************************************************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#define handle_error(msg) \
			{ perror(msg); \
				exit(1); }
 
struct acc
{
	double x,y,z, Roll,Pitch,Yaw;	
	struct timespec timestamp;	
}obj;

pthread_mutex_t lock;
void *thread1(void *);
void *thread2(void *);
double rand_double();
int flag;

void main()
{
	int rc;
	pthread_t t1,t2;
	
	srand(time(0));
	rc = pthread_mutex_init(&lock, NULL);
	if(rc!= 0)
		handle_error("Mutex init");
	
	rc = pthread_create(&t1,(void *)0,thread1,NULL);
	if(rc!= 0)
		handle_error("Pthread Create");
			
	rc = pthread_create(&t2,(void *)0,thread2,NULL);
	if(rc!= 0)
		handle_error("Pthread Create");
		
	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	
	rc = pthread_mutex_destroy(&lock);
	if(rc == -1)
		handle_error("mutex destroy");
}

void *thread1(void *arg)
{
	int rc1;
	
	while(1)
	{
		rc1 = pthread_mutex_lock(&lock);
		if(rc1!= 0)
			handle_error("Mutex lock");
		
		obj.x = rand_double();
		obj.y = rand_double();
		obj.z = rand_double();
		obj.Roll = rand_double();
		obj.Pitch = rand_double();
		obj.Yaw = rand_double();
		clock_gettime(CLOCK_REALTIME, &obj.timestamp);
		printf("[%ld : %ld : %ld : %ld] Updated: X=%f, Y=%f, Z=%f, Roll=%f, Pitch=%f, Yaw=%f\n",obj.timestamp.tv_sec, (obj.timestamp.tv_nsec / 1000000), (obj.timestamp.tv_nsec / 1000), obj.timestamp.tv_nsec,obj.x, obj.y, obj.z, obj.Roll, obj.Pitch, obj.Yaw);
		
		rc1 = pthread_mutex_unlock(&lock);
		if(rc1!= 0)
			handle_error("Mutex unlock");
		flag = 1;
		sleep(1);
	}
	pthread_exit(NULL);
}

double rand_double()
{
	return (((double)rand()/RAND_MAX)*71);
}


void *thread2(void *arg)
{
	while(1)
	{
		if(flag == 1)
		{
			pthread_mutex_lock(&lock);
			printf("[%ld : %ld : %ld : %ld] Received: X=%f, Y=%f, Z=%f, Roll=%f, Pitch=%f, Yaw=%f\n",obj.timestamp.tv_sec, (obj.timestamp.tv_nsec / 1000000), (obj.timestamp.tv_nsec / 1000), obj.timestamp.tv_nsec,obj.x, obj.y, obj.z,obj.Roll, obj.Pitch, obj.Yaw);
			pthread_mutex_unlock(&lock);
			flag = 0;
		}
	}
	pthread_exit(NULL);
}
