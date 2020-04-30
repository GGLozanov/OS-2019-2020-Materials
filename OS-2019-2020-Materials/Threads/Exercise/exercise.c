#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

int is_prime(int num) {
	for(int i = 2; i < num; i++) {
		if(num % i == 0) return 0;
	}
	return 1;
}

void* calculate_primes(void* arg) {
	int n = *((int*) arg);
	free(arg);
	int count = 0;
	printf("Prime calculation started for N=%d\n", n);
	for(int i = 2; i < n; i++) {
		if(is_prime(i)) count++;
	}
	printf("Number of primes for N=%d is %d\n", n, count);
}

int main() {
	char buffer[200];
	pthread_t threads[100];
	int thread_num = 0;
	while(1) {
		fgets(buffer, 199, stdin);
		if(strstr(buffer, "p ")) {
			char inner_buffer[100];
			int inner_buffer_index = 0;
			
			int* p = malloc(sizeof(int));
			for(int j = 2; j < strlen(buffer); j++) {
				inner_buffer[inner_buffer_index++] = buffer[j];
			}
			*p = atoi(inner_buffer);
			pthread_create(&threads[thread_num++], NULL, calculate_primes, p);
		} else if(*buffer == 'e') {
			for(int i = 0; i < thread_num; i++) {
				pthread_join(threads[i], NULL);
			}
			break;
		} else {
			printf("Supported commands:\np N - Starts a new calculation for the number of primes from 1 to N\ne - Waits for all calculations to finish and exits\n");
		}
	}
	
	return 0;
}
