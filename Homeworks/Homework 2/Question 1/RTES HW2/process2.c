#include <stdio.h>
#include <fcntl.h>           
#include <sys/stat.h> 
#include <semaphore.h>

void main()
{
		sem_t *sem1;
		sem1 = sem_open("/Amreeta",O_EXCL);
		if(sem1 == SEM_FAILED)
		{
			printf("sem_open failed!\n");
		}
		sem_unlink("/Amreeta");
		printf("Before semaphore is posted.\n");
		sem_post(sem1);
		printf("After semaphore is posted.\n");
		sem_close(sem1);
	
}
