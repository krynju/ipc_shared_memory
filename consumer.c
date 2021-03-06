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
	int q_id = 100;
	int iter = 10;
	key_t unique_key;
	int segment_id;
	int semaphore_id;
	int *shared_memory;
	const unsigned int shared_segment_size = 10 * sizeof(int);
	union semun arg;
	
	int previous_full_index;
	int read_number;
	
	if(argc == 4){
		iter = atol(argv[1]);
		p_id = atol(argv[2]);
		q_id = atol(argv[3]);
	}
	
	printf("running consumer process %d\n", p_id);
	
	unique_key = generate_key(q_id);
	semaphore_id = get_semaphore(unique_key);
	segment_id = get_shared_memory_segment_id(unique_key, shared_segment_size);
	shared_memory = get_shared_memory_handle(segment_id);
	
	printf("| %-5s| %-5s| %-5s| %-5s| %-5s| %-5s| %-5s| %-5s| %-5s| %-5s|\n", 
			"p_id","q_id","iter", "mutex", "empty", "full", "users", "oper", "@indx", "reslt");	
	
	semaphore_post(semaphore_id, USERS, SEM_UNDO);
	
	for(int i = 0; i<iter; i++){
		semaphore_wait(semaphore_id, FULLNESS, 0);
		semaphore_wait(semaphore_id, MUTEX, SEM_UNDO);
		
		previous_full_index = get_semaphore_value(semaphore_id, FULLNESS);
		read_number = shared_memory[previous_full_index];
		
		semaphore_post(semaphore_id, EMPTINESS, 0);
		
		semctl(semaphore_id, 0, GETALL, arg);
		printf("| %-5d| %-5d| %-5d| %-5d| %-5d| %-5d| %-5d| %-5s| %-5d| %-5d|\n", 
				p_id, q_id, i, arg.array[MUTEX], arg.array[EMPTINESS], arg.array[FULLNESS], 
				arg.array[USERS], "deq", previous_full_index, read_number);
		
		semaphore_post(semaphore_id, MUTEX, SEM_UNDO);
		
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

	printf("ending consumer process %d\n", p_id);
	
	return 0;
}