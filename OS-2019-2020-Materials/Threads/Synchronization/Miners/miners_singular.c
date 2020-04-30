#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define WORKER_COUNT 1
#define GOLD_MARGIN 10

long int gold_storage = 0;
pthread_mutex_t mutex;

void* mine(void* arg) {
	int id = *((int*) arg);

	for(int i = 0; i < 20; i++) {
		pthread_mutex_lock(&mutex);
		gold_storage += GOLD_MARGIN;
		printf("Miner %d gathered 10 gold\n", id);
		pthread_mutex_unlock(&mutex);
		sleep(2);
	}
}

void* sell(void* arg) {
	int id = *((int*) arg);

	for(int i = 0; i < 20; i++) {
		pthread_mutex_lock(&mutex);
		if(gold_storage == 0) {
			printf("The warehouse is empty, cannot sell!\n");
		}  else {
			gold_storage -= GOLD_MARGIN;
			printf("Trader %d sold 10 gold\n", id);
		}
		pthread_mutex_unlock(&mutex);
		sleep(2);
	}
}

int main(int argc, char** argv) {
	pthread_mutex_init(&mutex, NULL);
	pthread_t producer; // producer
	pthread_t consumer; // consumer
	
	int* tmp = malloc(sizeof(int));
	*tmp = 1;
	
	pthread_create(&consumer, NULL, sell, tmp);
	pthread_create(&producer, NULL, mine, tmp);
	
	pthread_join(consumer, NULL);
	pthread_join(producer, NULL);
	
	printf("Gold: %ld\n", gold_storage);
}
