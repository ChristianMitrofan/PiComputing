/*Program for computing pi*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <time.h>

union semun
{
    int val;
    struct semid_ds *buf;
    ushort *array;
};

void free_mem(int mem_id)
{
	shmctl(mem_id,IPC_RMID,NULL);
};

void destroy_sem(int sem_id)
{
	semctl(sem_id,0,IPC_RMID,0);
};

void sem_up(int sem_id,int sem_n)
{
	int error;
	struct sembuf buf;
	buf.sem_num=sem_n;
	buf.sem_op=1;
	buf.sem_flg=0;
	error=semop(sem_id,&buf,1);
	if(error==-1)
		printf("Semaphore couldn't be upped");
};

void sem_down(int sem_id,int sem_n)
{
	int error;
	struct sembuf buf;
	buf.sem_num=sem_n;
	buf.sem_op=-1;
	buf.sem_flg=0;
	error=semop(sem_id,&buf,1);
	if(error==-1)
		printf("Semaphore couldn't be downed");
};

int check_valid(float x,float y,float r)
{
	return ((x-r/2)*(x-r/2)+(y-r/2)*(y-r/2))<((r/2)*(r/2));
};

float random_point(int r)
{
	return (float)rand()/(float)(RAND_MAX/r);
};