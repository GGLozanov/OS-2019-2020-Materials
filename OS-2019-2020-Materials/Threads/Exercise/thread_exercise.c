#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* work(void* arg) {
	int id = *((int*) arg);
	free(arg);
	// int id = (int) arg;
	printf("Hello from thread %d \n", id);
	
	return (void *) (id * 2);
}

int main() {
	pthread_t threads[10]; // instantiate pthread_t variable
	for(int i = 0; i < 10; i++) {
		int* p = malloc(sizeof(int));
		*p = i;
		int indicator = pthread_create(&threads[i], NULL, work, p); // 4th arg -> function arguments
		// just giving function args is not viable because threads
		// give variables just as they are, not pointers
		// that way the compiler just reads the variable as bytes, not by reference, and works for a direct cast
		// this only works when int is exactly void* size
		
		// which is why casting to (void*) works or allocing memory as well works
		// or creating a struct works as well
		// passing a struct as one argument containing all the arguments you wish
	}
	
	// threads need to be joined in a separate loop from the creation thread
	
	for(int i = 0; i < 10; i++) {
		void* result; // ensure that you have enough memory
		pthread_join(threads[i], &result); // this is a dangerous move because the memory is of undefined size
		printf("Joined %d\n", (int)result);
	} 
	// need to join the thread in order for the routine to work 
	// (we don't wait for the thread and the main program executes before the thread routine)
}

