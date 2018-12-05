#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <helpers.h>

int main(int argc, char* argv[]){
	key_t unique_key;
	int segment_id;
	int semaphore_id;
	int *shared_memory;
	const unsigned int shared_segment_size = 10 * sizeof(int);
	
	unique_key = generate_key(420);	// Generate "unique" key 
	
	semaphore_id = initialise_semaphore(unique_key);

	segment_id = get_shared_memory_segment_id(unique_key, shared_segment_size);
	shared_memory = get_shared_memory_handle(segment_id);
	
	//code start
	semaphore_wait(semaphore_id, 1);
	shared_memory[0] = 420;
	printf("%d\n", shared_memory[0]);
	//code end
	
	shmdt(shared_memory);	// Detach the shared memory segment.
	union semun ignored_argument;
	semctl(semaphore_id, 1, IPC_RMID, ignored_argument);	// Detach semaphore
	shmctl(segment_id, IPC_RMID, 0);					// Release shared memory
	return 0;
}