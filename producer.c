#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <helpers.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	key_t unique_key;
	int segment_id;
	int semaphore_id;
	int *shared_memory;
	const unsigned int shared_segment_size = 10 * sizeof(int);
	
	unique_key = generate_key(421);	// Generate "unique" key 
	semaphore_id = initialise_semaphore(unique_key);
	segment_id = get_shared_memory_segment_id(unique_key, shared_segment_size);
	shared_memory = get_shared_memory_handle(segment_id);
	
	for(int i = 0; i<20; i++){
		semaphore_wait(semaphore_id, EMPTINESS);
		semaphore_wait(semaphore_id, MUTEX);
		// enqueue
		printf("enqueue\n");
		
		semaphore_post(semaphore_id, FULLNESS);
		semaphore_post(semaphore_id, MUTEX);
		
		union semun arg;
		semctl(semaphore_id, 0, GETALL,arg);
		printf("m=%2d e=%2d f=%2d\n", arg.array[0], arg.array[1], arg.array[2]);
		sleep(1);
	}
	
	shmdt(shared_memory);	// Detach the shared memory segment.
	union semun ignored_argument;
	semctl(semaphore_id, 0, IPC_RMID, ignored_argument);	// Detach semaphore
	shmctl(segment_id, IPC_RMID, 0);					// Release shared memory
	return 0;
}