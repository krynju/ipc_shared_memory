#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main(int argc, char* argv[]){
	key_t unique_key;
	int segment_id;
	int *shared_memory;
	const int shared_segment_size = 10 * sizeof(int);
	
	
	/* Generate "unique" key */
	unique_key = ftok("README.md", 420);
	if(unique_key == -1){
		perror("ftok: ");
		exit(1);
	}
	
	/* Allocate a shared memory segment. */
	segment_id = shmget(unique_key, shared_segment_size,
						IPC_CREAT | S_IRUSR | S_IWUSR);
						
	/* Attach the shared memory segment. */
	shared_memory = (int*)shmat(segment_id, 0, 0); 
	// SHM_RND not needed at 3rd argument, because address is null 
	// so the returned address will be free and aligned to page
	
	//printf("shared memory attached at address %p\n", shared_memory);
	shared_memory[0] = 69;
	
	/* Read from shared memory */
	printf("%d\n", shared_memory[0]);
	
	/* Detach the shared memory segment. */
	shmdt(shared_memory);
	
	/* Deallocate the shared memory segment. */
	shmctl(segment_id, IPC_RMID, 0);
	return 0;
}