#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>

int main(int argc, char* argv[]){
	key_t unique_key;
	
	int segment_id;
	char* shared_memory;
	struct shmid_ds shmbuffer;
	int segment_size;
	const int shared_segment_size = 0x6400;
	
	/* Generate "unique" key */
	unique_key = ftok("README.md", 420);
	if(unique_key == -1){
		perror("ftok: ");
		exit(1);
	}
	
	/* Allocate a shared memory segment. */
	segment_id = shmget(unique_key, shared_segment_size,
						IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	
	/* Attach the shared memory segment. */
	shared_memory = (char*)shmat(segment_id, 0, 0);
	printf ("shared memory attached at address %p\n", shared_memory);
	
	/* Determine the segment’s size. */
	// shmctl (segment_id, IPC_STAT, &shmbuffer);
	// segment_size = shmbuffer.shm_segsz;
	// printf ("segment size: %d\n", segment_size);
	
	/* Write a string to the shared memory segment. */
	sprintf(shared_memory, "Hello, world.");
	
	/* Read from shared memory */
	printf("%s\n", shared_memory);
	
	/* Detach the shared memory segment. */
	shmdt (shared_memory);
	
	
	/* Reattach the shared memory segment, at a different address. */
	// shared_memory = (char*) shmat (segment_id, (void*) 0x5000000, 0);
	// printf ("shared memory reattached at address %p\n", shared_memory);
	
	/* Print out the string from shared memory. */
	// printf ("%s\n", shared_memory);
	
	/* Detach the shared memory segment. */
	// shmdt (shared_memory);
	
	/* Deallocate the shared memory segment. */
	shmctl (segment_id, IPC_RMID, 0);
	return 0;
}