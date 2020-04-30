#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

#define ARRAY_SIZE 100000

long double sum = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // global since mutex and threads shouldn't access it

void* work(void* arg) {
	// optimal variant
	int i;
	long double tmp = 0;
	for(i = 0; i < ARRAY_SIZE/2; i++) {
		tmp += sin(i) * sin(i) + cos(i) * cos(i);
		printf("Thread 1: %Lf\n", sum);
	}
	pthread_mutex_lock(&mutex); // critical sections often have global vars so that's how you defend from race condition
	sum += tmp; // optimizing mutex is good (place slow operations in tmp variables)
	pthread_mutex_unlock(&mutex);
	return NULL;
}

void* work2(void* arg) {
	// unoptimal variant
	int i;
	for(i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
		pthread_mutex_lock(&mutex); // locks by allowing only one thread to go into that
		sum += sin(i) * sin(i) + cos(i) * cos(i);
		printf("Thread 2: %Lf\n", sum);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

int main() {

	pthread_t thread1;
	pthread_t thread2;
	
	pthread_create(&thread1, NULL, work, NULL);
	pthread_create(&thread2, NULL, work2, NULL);
	
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	printf("%Lf\n", sum);
}
