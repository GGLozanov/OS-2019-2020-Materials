//--------------------------------------------
// NAME: Georgi Lozanov
// CLASS: XIa
// NUMBER: 11
// PROBLEM: #3
// FILE NAME: starcraft3.c (unix file name)
// FILE PURPOSE:
// симулация на прототип на "Starcraft 3" 
// въз основа на multithreading и синхронизация с бинарни семафори (mutex-и)
//---------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#define INITIAL_SCV_COUNT 5
#define MARINE_TOTAL 20
#define MINERAL_MARGIN 8
#define UNIT_COST 50
#define UNIT_MAX 200
#define DEFAULT_MINERALS 1000
#define MINERAL_BLOCK_VALUE 500
#define DEFAULT_BLOCKS 2
#define ll long long

int total_mineral_blocks = DEFAULT_BLOCKS;

struct block_t {
	pthread_mutex_t mutex;
	ll value;
};

struct scv_t {
	int id;
	pthread_t scv;
};

struct command_center_t {
	unsigned ll player_minerals;
	unsigned int marine_count;
	unsigned int current_scv_count;
	unsigned ll map_minerals;
	pthread_mutex_t command_center_mutex;
	struct scv_t scvs[UNIT_MAX];
};

struct command_center_t command_center;
struct block_t* blocks;

//--------------------------------------------
// FUNCTION: calc_map_minerals (име на функцията)
// calculates the total map minerals
// PARAMETERS:
// none
//----------------------------------------------

ll calc_map_minerals() {
	return command_center.player_minerals + ((command_center.current_scv_count - INITIAL_SCV_COUNT) * UNIT_COST) + (command_center.marine_count * UNIT_COST);
}

//--------------------------------------------
// FUNCTION: minerals_cond (име на функцията)
// exit condition for minerals that avoids race condition with mutex synchronization
// PARAMETERS:
// none
//----------------------------------------------

int minerals_cond() {
	pthread_mutex_lock(&command_center.command_center_mutex);
	int is_cond = command_center.map_minerals > 0;
	pthread_mutex_unlock(&command_center.command_center_mutex);
	return is_cond;
}

//--------------------------------------------
// FUNCTION: minerals_cond (име на функцията)
// exit condition for marines
// PARAMETERS:
// none
//----------------------------------------------

int marine_cond() {
	return command_center.marine_count < MARINE_TOTAL;
}

//--------------------------------------------
// FUNCTION: be_scv (име на функцията)
// main work routine for the worker threads
// PARAMETERS:
// void* args - work routine arguments garnered through casting inside the thread routine
//----------------------------------------------

void* be_scv(void* args) {
	struct scv_t* scv = (struct scv_t*) args;

	// local tracking of freed mineral blocks
	// outer loop for soldier check?
	while(1) {
		for(int i = 0; i < total_mineral_blocks; i++) { 
			if(pthread_mutex_trylock(&blocks[i].mutex) != EBUSY) {
			    sleep(3);
				int is_lower_than_margin = 0;
				int minerals_taken = MINERAL_MARGIN;
				if(blocks[i].value > 0) {
					printf("SCV %d is mining from mineral block %d\n", scv->id, i + 1);
					is_lower_than_margin = blocks[i].value - MINERAL_MARGIN < 0 ? 1 : 0;
					if(is_lower_than_margin) minerals_taken = MINERAL_MARGIN - blocks[i].value;
					command_center.map_minerals -= minerals_taken;
					blocks[i].value = is_lower_than_margin ? 0 : blocks[i].value - minerals_taken;
					pthread_mutex_unlock(&blocks[i].mutex);

					printf("SCV %d is transporting minerals\n", scv->id);
					sleep(2);
					
					pthread_mutex_lock(&command_center.command_center_mutex);
					printf("SCV %d delivered minerals to the Command center\n", scv->id);
					command_center.player_minerals += minerals_taken;
					pthread_mutex_unlock(&command_center.command_center_mutex);
				}
			} else sleep(3);
			if(!minerals_cond()) break;
		}
		if(!minerals_cond()) break;
	}
	
	return NULL;	
}

//--------------------------------------------
// FUNCTION: can_buy (име на функцията)
// checks if the player can buy a given unit
// PARAMETERS:
// none
//----------------------------------------------

int can_buy() {
	return command_center.player_minerals >= UNIT_COST;
}

//--------------------------------------------
// FUNCTION: train_marine (име на функцията)
// trains one marine
// PARAMETERS:
// none
//----------------------------------------------

void train_marine() {
	command_center.player_minerals -= UNIT_COST;
	sleep(1);
	printf("You wanna piece of me, boy?\n");
	command_center.marine_count++;
}

//--------------------------------------------
// FUNCTION: buy_marine (име на функцията)
// buys and trains marine
// PARAMETERS:
// none
//----------------------------------------------

void buy_marine() {
	if(can_buy()) {
		train_marine();
	} else printf("Not enough minerals.\n");
}

//--------------------------------------------
// FUNCTION: buy_marine (име на функцията)
// creates the scv thread with a given current index
// PARAMETERS:
// int current_scv_index - the current scv index
//----------------------------------------------

void create_scv(int current_scv_index) {
	struct scv_t* scv = &command_center.scvs[current_scv_index];
	scv->id = current_scv_index + 1;
	if(pthread_create(&command_center.scvs[current_scv_index].scv, NULL, be_scv, (void*) scv) != 0) {
		perror("");
		exit(0);
	}
}

//--------------------------------------------
// FUNCTION: buy_marine (име на функцията)
// buys and creates an scv with a cost of UNIT_COST
// PARAMETERS:
// none
//----------------------------------------------

void buy_scv() {
	if(can_buy()) {
		command_center.player_minerals -= UNIT_COST;
		sleep(4);
		printf("SCV good to go, sir.\n");
		command_center.current_scv_count++;
		create_scv(command_center.current_scv_count);
	} else printf("Not enough minerals.\n");
}

//--------------------------------------------
// FUNCTION: init_scv (име на функцията)
// initialises an scv struct (without the thread)
// PARAMETERS:
// int id - the current scv's id
//----------------------------------------------

struct scv_t init_scv(int id) {
	struct scv_t scv = {id};
	return scv;
}

//--------------------------------------------
// FUNCTION: init_scvs (име на функцията)
// initialises an array of scv structs (without the threads)
// PARAMETERS:
// none
//----------------------------------------------

void init_scvs() {
	for(int i = 0; i < UNIT_MAX; i++) {
		command_center.scvs[i] = init_scv(i);
	}
}

//--------------------------------------------
// FUNCTION: init_command_center (име на функцията)
// initialises the player command center struct (without the worker threads)
// PARAMETERS:
// unsigned ll map_minerals - the initial value of the map minerals
//----------------------------------------------

struct command_center_t init_command_center(unsigned ll map_minerals) {
	struct command_center_t cmd_ctr = {0, 0, INITIAL_SCV_COUNT, map_minerals, PTHREAD_MUTEX_INITIALIZER};
	init_scvs();
	return cmd_ctr;
}

//--------------------------------------------
// FUNCTION: play (име на функцията)
// the main work routine for the player, handling all input
// PARAMETERS:
// void* args - the work routine argument/s (none, in this case)
//----------------------------------------------

void* play(void* args) {
	int ch; // keep digging if there are minerals even if soldiers have been bought
	while(marine_cond()) {
		if(command_center.marine_count + command_center.current_scv_count < UNIT_MAX) {
			ch = getchar();
			switch(ch) {
				case 'm':
					pthread_mutex_lock(&command_center.command_center_mutex);
					buy_marine();
					pthread_mutex_unlock(&command_center.command_center_mutex);
					break;
				case 's':
					pthread_mutex_lock(&command_center.command_center_mutex);
					buy_scv();
					pthread_mutex_unlock(&command_center.command_center_mutex);
					break;
			}
		}
	}
	
	return NULL;
}

//--------------------------------------------
// FUNCTION: main (име на функцията)
// the main function of execution, handling the creation and joining of threads, the initialisation and destruction of mutexes
// PARAMETERS:
// int argc - the cout of the given console arguments
// char** argv - the actual console arguments contained in a 2D array
//----------------------------------------------

int main(int argc, char** argv) {
	ll initial_map_minerals = DEFAULT_MINERALS;
	if(argc > 1) {
		total_mineral_blocks = atoi(argv[1]);
		initial_map_minerals = total_mineral_blocks * MINERAL_BLOCK_VALUE;
	}
	
	int i = 0;
	
	command_center = init_command_center(initial_map_minerals);
	blocks = (struct block_t*) malloc(sizeof(struct block_t) * total_mineral_blocks);
	
	for(; i < total_mineral_blocks; i++) {
		if(pthread_mutex_init(&blocks[i].mutex, NULL) != 0) {
			perror("");
			exit(1);
		}
		blocks[i].value = MINERAL_BLOCK_VALUE;
	}
	
	for(i = 0; i < INITIAL_SCV_COUNT; i++) {
		create_scv(i);
	}
	
	pthread_t player;
	if(pthread_create(&player, NULL, play, NULL) != 0) {
		perror("");
		exit(0);
	}

	if(pthread_join(player, NULL) != 0) {
		perror("");
		exit(2);
	}

	for(i = 0; i < command_center.current_scv_count; i++) {
		if(pthread_join(command_center.scvs[i].scv, NULL) != 0) {
			perror("");
			exit(2);
		}
	}
	
	for(i = 0; i < total_mineral_blocks; i++) {
		 pthread_mutex_destroy(&blocks[i].mutex);
	}
	
	free(blocks);

	pthread_mutex_destroy(&command_center.command_center_mutex);

	printf("Map minerals %lld, player minerals %lld, SCVs %d, Marines %d\n", calc_map_minerals(), command_center.player_minerals, command_center.current_scv_count, command_center.marine_count);
	
}
