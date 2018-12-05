#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

key_t generate_key(int secondary_id){
	key_t unique_key = ftok("README.md", secondary_id);
	if(unique_key == -1){
		perror("ftok: ");
		exit(1);
	}
	return unique_key;
}

int get_shared_memory_segment_id(int key, unsigned int size){
	/* Allocate a shared memory segment. */
	int segment_id;
	segment_id = shmget(key, size, IPC_CREAT | S_IRUSR | S_IWUSR);
	// todo: add error handling
	return segment_id;
}

int *get_shared_memory_handle(int segment_id){
	int *shared_memory;
	/* Attach the shared memory segment. */
	shared_memory = (int*)shmat(segment_id, 0, 0); 
	// todo: add error handling
	return shared_memory;
}

int semaphore_wait(int semid, int semaphore_num){
	struct sembuf operations[1]; /* Use the first (and only) semaphore. */
	operations[0].sem_num = semaphore_num;
	operations[0].sem_op = -1; /* Decrement by 1. */
	operations[0].sem_flg = SEM_UNDO; /* Permit undo’ing. */
	return semop (semid, operations, 1);
}

int binary_semaphore_post (int semid, int semaphore_num)
{
	struct sembuf operations[1];
	operations[0].sem_num = semaphore_num;	// choose the semaphore
	operations[0].sem_op = 1;				/* Increment by 1. */
	operations[0].sem_flg = SEM_UNDO;		/* Permit undo’ing. */
	return semop (semid, operations, 1);
}

int initialise_semaphore(int key){
	int semaphore_id = semget(key, 3, IPC_CREAT | S_IRUSR | S_IWUSR); // Allocate semaphore
	
	/* Initialise a semaphore */
	union semun argument;
	unsigned short values[] = {1,10,0};
	argument.array = values;
	semctl(semaphore_id, 0, SETALL, argument);	// init values
	return semaphore_id;
}


