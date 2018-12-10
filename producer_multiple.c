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
	int q_id[5] = {100, 101, 102, 103, 104};
	int iter = 10;
	key_t unique_key[5];
	int segment_id[5];
	int semaphore_id[5];
	int *shared_memory[5];
	const unsigned int shared_segment_size = 10 * sizeof(int);
	union semun arg;
	
	int next_empty_index;
	int rand_number;
	
	if(argc == 3){
		iter = atol(argv[1]);
		p_id = atol(argv[2]);
	}
	if(argc == 8){
		iter = atol(argv[1]);
		p_id = atol(argv[2]);
		q_id[0] = atol(argv[3]);
		q_id[1] = atol(argv[4]);
		q_id[2] = atol(argv[5]);
		q_id[3] = atol(argv[6]);
		q_id[4] = atol(argv[7]);
	}
	
	printf("running producer process %d\n", p_id);
	
	for(int i = 0; i<5 ; i++){
		unique_key[i] = generate_key(q_id[i]);	// Generate "unique" key 
		semaphore_id[i] = get_semaphore(unique_key[i]);
		segment_id[i] = get_shared_memory_segment_id(unique_key[i], shared_segment_size);
		shared_memory[i] = get_shared_memory_handle(segment_id[i]);
		semaphore_post(semaphore_id[i], USERS, SEM_UNDO);
	}
	
	printf("| %-5s| %-5s| %-5s| %-5s| %-5s| %-5s| %-5s| %-5s| %-5s| %-5s|\n", 
			"p_id","q_id","iter", "mutex", "empty", "full", "users", "oper", "@indx", "reslt");	
			
	int queue = 0;
	
	for(int i = 0; i<iter; i++){		
		if(-1 == semaphore_wait_no_errorcheck(semaphore_id[queue], EMPTINESS, IPC_NOWAIT)){
			if(errno == EAGAIN){
				--i;
				++queue;
				queue = queue%5;
				continue;
			}
		}
		
		semaphore_wait(semaphore_id[queue], MUTEX, SEM_UNDO);

		next_empty_index = get_semaphore_value(semaphore_id[queue], FULLNESS);
		rand_number = rand()%32;
		shared_memory[queue][next_empty_index] = rand_number;
		
		semaphore_post(semaphore_id[queue], FULLNESS, 0);
		
		printf("| %-5d| %-5d| %-5d| %-5d| %-5d| %-5d| %-5d| %-5s| %-5d| %-5d|\n", 
				p_id, q_id[queue], i, get_semaphore_value(semaphore_id[queue], MUTEX), 
				get_semaphore_value(semaphore_id[queue], EMPTINESS), 
				get_semaphore_value(semaphore_id[queue], FULLNESS), 
				get_semaphore_value(semaphore_id[queue], USERS), "enq", 
				next_empty_index, rand_number);
				
		semaphore_post(semaphore_id[queue], MUTEX, SEM_UNDO);
	
		sleep(1);
		++queue;
		queue = queue%5;
	}
	
	for(int i = 0; i<5 ; i++){
		shmdt(shared_memory[i]);					// Detach the shared memory segment.
		semaphore_wait(semaphore_id[i], MUTEX, SEM_UNDO);
		semaphore_wait(semaphore_id[i], USERS, SEM_UNDO);
		
		if(get_semaphore_value(semaphore_id[i], USERS) == 0){
			detach_semaphore(semaphore_id[i]);
			shmctl(segment_id[i], IPC_RMID, 0);					// Release shared memory
		}
		else
			semaphore_post(semaphore_id[i], MUTEX, SEM_UNDO);
	}
	
	printf("ending producer process %d\n", p_id);

	return 0;
}