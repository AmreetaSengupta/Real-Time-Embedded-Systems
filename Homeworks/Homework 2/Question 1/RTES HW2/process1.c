#include <stdio.h>
#include <fcntl.h>           
#include <sys/stat.h> 
#include <semaphore.h>

void main()
{
		sem_t *sem1;
		sem1 = sem_open("/Amreeta",O_CREAT,0777,0);
		if(sem1 == SEM_FAILED)
		{
			printf("sem_open failed!\n");
		}
		printf("Waiting for semaphore to post.\n");
		sem_wait(sem1);
		printf("Semaphore posted.\n");
		sem_close(sem1);
	
}
