#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 3

void* print(void* null) { // func always needs to return void* and accept void* args; casting otherwise
	printf("thread bebyyyy");
	pthread_exit((void *) 0);
}

int main() {
	pthread_t thread[NUM_THREADS];
	int indicator;
	
	for(int i = 0; i < NUM_THREADS; i++) {
		indicator = pthread_create(&thread[i], NULL, print, NULL); // returns 1 if failed
		// casting when giving arguments in pthread_create()
		if(indicator) {
			
		}
	}
	
	// new thread doesn't know when the function will be executed (or which after which)
	
	return 0;
}
