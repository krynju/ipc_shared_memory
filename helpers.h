#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

enum sem_type {MUTEX, EMPTINESS, FULLNESS, USERS};

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

key_t generate_key(int secondary_id){
	key_t unique_key = ftok(".", secondary_id);
	if(unique_key == -1){
		perror("ftok: ");
		exit(1);
	}
	return unique_key;
}

int get_shared_memory_segment_id(int key, unsigned int size){
	/* Allocate a shared memory segment. */
	int segment_id = shmget(key, size, IPC_CREAT | S_IRUSR | S_IWUSR);
	if(segment_id == -1){
		perror("shmget: ");
		exit(1);
	}
	return segment_id;
}

int *get_shared_memory_handle(int segment_id){
	int *shared_memory;
	/* Attach the shared memory segment. */
	shared_memory = (int*)shmat(segment_id, 0, 0); 
	if(segment_id == -1){
		perror("shmat: ");
		exit(1);
	}
	return shared_memory;
}

int get_semaphore_value(int semid, int semnum){
	union semun arg;
	return semctl(semid, semnum, GETVAL, arg);
}

int semaphore_wait(int semid, int semaphore_num, int flags){
	struct sembuf operations[1];
	operations[0].sem_num = semaphore_num;	// choose the semaphore
	operations[0].sem_op = -1; 				// decrement value
	operations[0].sem_flg = flags;			// add flags - sometimes SEM_UNDO
	int result = semop (semid, operations, 1);
	if(result == -1){
		perror("semop_post: ");
		exit(1);
	}
	return result;
}

int semaphore_wait_no_errorcheck(int semid, int semaphore_num, int flags){
	struct sembuf operations[1];
	operations[0].sem_num = semaphore_num;	// choose the semaphore
	operations[0].sem_op = -1; 				// decrement value
	operations[0].sem_flg = flags;			// add flags - sometimes SEM_UNDO
	return semop (semid, operations, 1);;
}

int semaphore_post(int semid, int semaphore_num, int flags)
{
	struct sembuf operations[1];
	operations[0].sem_num = semaphore_num;	// choose the semaphore
	operations[0].sem_op = 1;				// increment value
	operations[0].sem_flg = flags;			// add flags - sometimes SEM_UNDO
	int result = semop (semid, operations, 1);
	if(result == -1){
		perror("semop_post: ");
		exit(1);
	}
	return result;
}

int initialise_semaphore(int semaphore_id){
	union semun argument;
	unsigned short values[] = {1,10,0,0};
	argument.array = values;
	if(semctl(semaphore_id, 0, SETALL, argument) == -1){
		perror("semctl SETALL: ");
		exit(1);
	}
}

int get_existing_semaphore(int key){
	int semaphore_id = semget(key, 4,  0);
	if(semaphore_id == -1){	
		perror("semget: ");
		exit(1);
	}
	return semaphore_id;
}

int get_semaphore(int key){
	int semaphore_id = semget(key, 4, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	if(semaphore_id == -1){	
		if(errno == EEXIST){
			return get_existing_semaphore(key);
		}else{
			perror("semget: ");
			exit(1);
		}
	}
	initialise_semaphore(semaphore_id);

	return semaphore_id;
}

int detach_semaphore(int semaphore_id){
	union semun arg;
	semctl(semaphore_id, 0, IPC_RMID, arg);
}




