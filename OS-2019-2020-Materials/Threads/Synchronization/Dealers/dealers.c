#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define DRIVER_AMOUNT 20
#define CAR_AMOUNT 5

pthread_mutex_t cars[CAR_AMOUNT];
pthread_t drivers[DRIVER_AMOUNT];

void* test_automobiles(void* args) {
	int id = *((int*) args);
	free(args);
	
	for(int i = 0; i < CAR_AMOUNT; i++) { // one driver tests multiple cars
		if(pthread_mutex_trylock(&cars[i]) != 0) { // if the mutex is busy. . .
			printf("Buyer %d takes car %d.\n", id, i + 1);
			printf("Buyer %d returns car %d.\n", id, i + 1);
			pthread_mutex_unlock(&cars[i]); // free the mutex for later. . .
		}
	}
	
	return NULL;
}

int main() {

	int i = 0;
	for(i; i < CAR_AMOUNT; i++) {
		pthread_mutex_init(&cars[i], NULL);
	}
	
	for(i = 0; i < DRIVER_AMOUNT; i++) {
		int* tmp = malloc(sizeof(int));
		*tmp = i + 1;
		pthread_create(&drivers[i], NULL, test_automobiles, tmp);
	}
	
	for(i = 0; i < DRIVER_AMOUNT; i++) {
		pthread_join(drivers[i], NULL);
	}
	
	for(i = 0; i < CAR_AMOUNT; i++) {
		pthread_mutex_destroy(&cars[i]);
	}
}
