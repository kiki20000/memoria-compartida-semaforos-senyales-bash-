#include <sys/types.h> 
#include <sys/ipc.h>   
#include <sys/sem.h>   
#include <sys/shm.h>  
#include <stdlib.h>
#include <stdio.h>

#define PERMISOS 0644

typedef union semun { 
    int val;              /* used for SETVAL only */ 
    struct semid_ds *buf; /* for IPC_STAT and IPC_SET */ 
    ushort *array;        /* used for GETALL and SETALL */ 
    } semun;

int sGet(int seed)
{
	int semID;
	key_t semKey = ftok(".", seed);

	if ((semID = semget(semKey, 1,IPC_CREAT | PERMISOS)) == -1) {
		perror("semget");
		exit(1);
	}

	//int semID = semget(semKey,1,IPC_CREAT | PERMISOS);
	return semID;
}

void sSet(int semID, int value)
{
	semctl(semID, 0, SETVAL, value);
}

int sCreate(int seed, int value)
{
	int semID = sGet(seed);
	//printf("**seed=%i**\n",semID);
	sSet(semID, value);
	return semID;
}

void sWait(int semID)
{
	struct sembuf op_Wait [] = 	{ 0, -1, 0 }; // Decrementa en 1 el semáforo 
	semop ( semID, op_Wait, 1 );
}

void sSignal(int semID)
{
	struct sembuf op_Signal [] = { 0, 1, 0 };	// Incrementa en 1 el semáforo 
	semop ( semID, op_Signal, 1 );
}


void sDestroy(int semID)
{
    semun dummy; 
    int ret;
    ret=semctl(semID, 0, IPC_RMID, dummy); 
    if (ret == -1 ) { perror("Error al obtener el semID: "); exit(-1);}
}