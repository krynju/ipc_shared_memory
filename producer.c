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
	int p_id = 0;
	int s_id = 100;
	key_t unique_key;
	int segment_id;
	int semaphore_id;
	int *shared_memory;
	const unsigned int shared_segment_size = 10 * sizeof(int);
	union semun arg;
	
	int next_empty_index;
	int rand_number;
	
	unique_key = generate_key(s_id);	// Generate "unique" key 
	semaphore_id = get_semaphore(unique_key);
	segment_id = get_shared_memory_segment_id(unique_key, shared_segment_size);
	shared_memory = get_shared_memory_handle(segment_id);
	
	printf("| %-5s\t| %-5s\t| %-5s\t| %-5s\t| %-5s\t| %-5s\t| %-5s\t| %-5s\t|\n", 
			"iter", "mutex", "empty", "full", "users", "oper", "@indx", "reslt");
			
	semaphore_post(semaphore_id, USERS, SEM_UNDO);
	
	for(int i = 0; i<7; i++){
		semaphore_wait(semaphore_id, EMPTINESS, 0);
		semaphore_wait(semaphore_id, MUTEX, SEM_UNDO);

		next_empty_index = get_semaphore_value(semaphore_id, FULLNESS);
		rand_number = rand()%32;
		shared_memory[next_empty_index] = rand_number;
		
		semaphore_post(semaphore_id, FULLNESS, 0);
		semaphore_post(semaphore_id, MUTEX, SEM_UNDO);
		
		semctl(semaphore_id, 0, GETALL, arg);
		printf("| %-5d\t| %-5d\t| %-5d\t| %-5d\t| %-5d\t| %-5s\t| %-5d\t| %-5d\t|\n", 
				i, arg.array[MUTEX], arg.array[EMPTINESS], arg.array[FULLNESS], 
				arg.array[USERS], "enq", next_empty_index, rand_number);
		sleep(1);
	}
	
	shmdt(shared_memory);					// Detach the shared memory segment.
	semaphore_wait(semaphore_id, MUTEX, SEM_UNDO);
	semaphore_wait(semaphore_id, USERS, SEM_UNDO);
	
	if(get_semaphore_value(semaphore_id, USERS) == 0){
		detach_semaphore(semaphore_id);
		shmctl(segment_id, IPC_RMID, 0);					// Release shared memory
	}
	else
		semaphore_post(semaphore_id, MUTEX, SEM_UNDO);

	return 0;
}