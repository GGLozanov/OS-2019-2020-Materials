#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define WORKER_COUNT 1
#define GOLD_MARGIN 10

long int gold_storage = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
	int producer_count = WORKER_COUNT, consumer_count = WORKER_COUNT;
	if(argc > 2) {
		producer_count = atoi(argv[1]);
		consumer_count = atoi(argv[2]);
	}
	pthread_t producer[producer_count]; // producer
	pthread_t consumer[consumer_count]; // consumer
	
	int* tmp;
	int i;
	
	for(i = 0; i < consumer_count; i++) {
		tmp = malloc(sizeof(int));
		*tmp = i + 1;
		pthread_create(&consumer[i], NULL, sell, tmp);
	}

	for(i = 0; i < producer_count; i++) {
		tmp = malloc(sizeof(int));
		*tmp = i + 1;
		pthread_create(&producer[i], NULL, mine, tmp);
	}
	
	for(i = 0; i < consumer_count; i++) {
		pthread_join(consumer[i], NULL);
	}
	
	for(i = 0; i < producer_count; i++) {
		pthread_join(producer[i], NULL);
	}

	
	printf("Gold: %ld", gold_storage);
}
