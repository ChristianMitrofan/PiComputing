/*Program for computing pi*/

#include "picomputing.h"

int main(int argc,char * argv[])
{
	srand(time(NULL));
	int tries;
	tries=atoi(argv[argc-1]); 								/*Number of tries is the last argument of main*/
	int i;													/*Used for iterations*/
	int n=argc-2;											/*Number of kids (and number of cubes given) are the number of arguments without the function name and the last argument(number of tries)*/
	int mem_id;
	int sem_id;
	int *buffer;											/*Mother writes in buffer[0](position of cube).Children return their values in buffer[1](length of cube) and buffer[2](1 for success,0 for failure),*/
	int ret_array[n][3];									/*First row has the length of the cube.Second one has the number of successful tries for every cube,and the third one has all the tries for each cube*/
	int cube_pos;											/*Position of the cube in ret_array*/
	int cube;												/*The length of the cube which the child uses and returns back*/
	int check;												/*1 if point(x,y) was in the circle,else 0*/
	float pointx,pointy;									/*Point(x,y)*/
	float s_rate;											/*Success rate: Oi=4*Ki/Ti*/
	for (i=0; i<n; i++)
	{
		ret_array[i][0]=atoi(argv[i+1]);
		ret_array[i][1]=0;
		ret_array[i][2]=0;
	}
/*											*
 * 	 	MEMORY ALLOCATION AND MAPPING		*
 *											*/
	mem_id=shmget(IPC_PRIVATE,sizeof(int)*4,IPC_CREAT | 0660);		/*Allocate*/
	buffer=(int *)shmat(mem_id,NULL,0);								/*and map a shared memory segment*/
	if(buffer==NULL)
	{
		printf("Memory was not allocated");
		free_mem(mem_id);
	}
	buffer[3]=1;													/*Buffer is in the shared memory*/
/*											*
 *	SEMAPHORES CREATION AND INSTATIATION	*
 *											*/
	union semun arg;
	sem_id=semget(1234,4,0660|IPC_CREAT);
	int er0,er1,er2,er3;
	arg.val=0;
	er0=semctl(sem_id,0,SETVAL,arg);						/*SEM0:buffer[0] is full*/
	er1=semctl(sem_id,1,SETVAL,arg);						/*SEM1:buffer[1] and buffer[2] are full*/
	arg.val=1;
	er2=semctl(sem_id,2,SETVAL,arg);						/*SEM2:buffer[0] is empty*/
	er3=semctl(sem_id,3,SETVAL,arg);						/*SEM3:buffer[1] and buffer[2] are empty*/
	if(er0==-1||er1==-1||er2==-1||er3==-1)
	{
		printf("Semaphores weren't initialised");
		destroy_sem(sem_id);
	}

	int m_id=getpid();
/*											*
 *  	CHILDREN PROCESSES CREATION			*
 *											*/
	for(i=0; i<n; i++)
	{
		fork();
		if (m_id!=getpid())
			break;
	}
/*											*
 *				MOTHER PROCESS				*
 *											*/
	if(m_id==getpid())
	{
		for(i=0; i<tries; i++)
		{
			cube_pos=rand()%n;							/*Choose a random position from ret_array and write it in the shared memory*/
			sem_down(sem_id,2);
			buffer[0]=cube_pos;
			sem_up(sem_id,0);
			sem_down(sem_id,1);
			check=buffer[2];
			cube_pos=buffer[1];
			sem_up(sem_id,3);
			ret_array[cube_pos][2]++;
			if(check==1)
			{
				ret_array[cube_pos][1]++;
			}
		}
		buffer[3]=0;
		sem_up(sem_id,0);
		for(i=0; i<n; i++)
		{
			wait();
		}
		for(i=0; i<n; i++)
		{
			s_rate=4*(float)ret_array[i][1]/(float)ret_array[i][2];
			printf("For the cube with a length of %d it was tested that of the %d random points %d were in the circle.So o of %d is %.6f\n",ret_array[i][0],ret_array[i][2],ret_array[i][1],ret_array[i][0],s_rate);
		}
		destroy_sem(sem_id);
		free_mem(mem_id);
	}
/*											*
 *			CHILDREN PROCESSES				*
 *											*/
	else
	{
		while(1)										
		{
			sem_down(sem_id,0);
			if(buffer[3]==0)
			{
				sem_up(sem_id,0);
				exit(EXIT_SUCCESS);
			}
			cube_pos=buffer[0];
			cube=ret_array[cube_pos][0];
			sem_up(sem_id,2);
			pointx=random_point(cube);
			pointy=random_point(cube);
			check=check_valid(pointx,pointy,(float)cube);
			sem_down(sem_id,3);
			buffer[1]=cube_pos;
			buffer[2]=check;
			sem_up(sem_id,1);
		}
	}
}